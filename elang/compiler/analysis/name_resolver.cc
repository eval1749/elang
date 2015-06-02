// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/name_resolver.h"

#include <unordered_set>

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// NameResolver::ReferenceResolver
//
class NameResolver::ReferenceResolver final : public Analyzer,
                                              private ast::Visitor {
 public:
  explicit ReferenceResolver(NameResolver* name_resolver,
                             ast::ContainerNode* container);
  ~ReferenceResolver() final = default;

  sm::Semantic* Resolve(ast::Expression* expression);

 private:
  void FindInClass(Token* name,
                   sm::Class* clazz,
                   std::unordered_set<sm::Semantic*>* founds);
  void ProduceResult(sm::Semantic* result);

  // ast::Visitor
  void VisitMemberAccess(ast::MemberAccess* node) final;
  void VisitNameReference(ast::NameReference* node) final;
  void VisitTypeMemberAccess(ast::TypeMemberAccess* node) final;
  void VisitTypeNameReference(ast::TypeNameReference* node) final;

  ast::ContainerNode* const container_;
  sm::Semantic* result_;

  DISALLOW_COPY_AND_ASSIGN(ReferenceResolver);
};

NameResolver::ReferenceResolver::ReferenceResolver(
    NameResolver* name_resolver,
    ast::ContainerNode* container)
    : Analyzer(name_resolver), container_(container), result_(nullptr) {
}

void NameResolver::ReferenceResolver::FindInClass(
    Token* name,
    sm::Class* clazz,
    std::unordered_set<sm::Semantic*>* founds) {
  if (auto const present = clazz->FindMember(name)) {
    founds->insert(present);
    return;
  }
  for (auto const base_class : clazz->direct_base_classes())
    FindInClass(name, base_class, founds);
}

void NameResolver::ReferenceResolver::ProduceResult(sm::Semantic* result) {
  DCHECK(!result_) << *result_;
  result_ = result;
}

sm::Semantic* NameResolver::ReferenceResolver::Resolve(
    ast::Expression* expression) {
  DCHECK(!result_) << *result_;
  Traverse(expression);
  return result_;
}

// ast::Visitor
void NameResolver::ReferenceResolver::VisitMemberAccess(
    ast::MemberAccess* node) {
  auto const container =
      resolver()->ResolveReference(node->container(), container_);
  if (!container) {
    ProduceResult(nullptr);
    return;
  }

  auto const semantic = container->FindMember(node->member());
  if (!semantic) {
    Error(ErrorCode::NameResolutionMemberAccessNotFound, node);
    ProduceResult(nullptr);
    return;
  }
  ProduceResult(semantic);
}

// Algorithm of this function should be equivalent to
// |ClassTreeBuilder::ResolveNameReference()|.
void NameResolver::ReferenceResolver::VisitNameReference(
    ast::NameReference* node) {
  auto const name = node->name();
  if (name->is_type_name()) {
    // type keyword is mapped into |System.XXX|.
    auto const clazz =
        session()->semantic_factory()->system_namespace()->FindMember(
            session()->PredefinedNameOf(name->mapped_type_name()));
    if (clazz) {
      ProduceResult(clazz);
      return;
    }
    Error(ErrorCode::NameResolutionNameNotFound, node);
    ProduceResult(session()->semantic_factory()->NewUndefinedType(name));
    return;
  }

  for (ast::Node* runner = container_; runner; runner = runner->parent()) {
    auto const container = SemanticOf(runner);
    if (!container) {
      DCHECK(runner->is<ast::Method>()) << runner;
      continue;
    }

    std::unordered_set<sm::Semantic*> founds;

    // Find in container
    if (auto const present = container->FindMember(name))
      founds.insert(present);

    if (auto const clazz = container->as<sm::Class>()) {
      if (founds.empty())
        FindInClass(name, clazz, &founds);

    } else if (auto const ns_body = runner->as<ast::NamespaceBody>()) {
      DCHECK(container->is<sm::Namespace>());
      // Find alias
      if (auto const alias = ns_body->FindAlias(name)) {
        if (auto const present = resolver()->RealNameOf(alias))
          founds.insert(present);
      }

      if (founds.empty()) {
        // When |name| isn't defined in namespace body, looking in imported
        // namespaces.
        resolver()->FindWithImports(name, ns_body, &founds);
      }
    } else {
      DCHECK(container->is<sm::Enum>() || container->is<sm::Method>())
          << container << " runner=" << runner;
    }

    if (founds.size() == 1)
      return ProduceResult(*founds.begin());

    if (founds.size() > 1) {
      Error(ErrorCode::NameResolutionNameAmbiguous, node->token(),
            (*founds.begin())->token());
      return;
    }
  }

  Error(ErrorCode::NameResolutionNameNotFound, node);
}

void NameResolver::ReferenceResolver::VisitTypeMemberAccess(
    ast::TypeMemberAccess* node) {
  VisitMemberAccess(node->reference());
}

void NameResolver::ReferenceResolver::VisitTypeNameReference(
    ast::TypeNameReference* node) {
  VisitNameReference(node->reference());
}

//////////////////////////////////////////////////////////////////////
//
// NameResolver
//
NameResolver::NameResolver(CompilationSession* session)
    : CompilationSessionUser(session) {
}

NameResolver::~NameResolver() {
}

sm::Factory* NameResolver::factory() const {
  return session()->semantic_factory();
}

void NameResolver::FindWithImports(Token* name,
                                   ast::NamespaceBody* ns_body,
                                   std::unordered_set<sm::Semantic*>* founds) {
  for (auto const pair : ns_body->imports()) {
    auto const imported_ns = ImportedNamespaceOf(pair.second);
    if (!imported_ns)
      continue;
    auto const present = imported_ns->FindMember(name);
    if (!present)
      continue;
    if (present->is<sm::Namespace>()) {
      // Import directive doesn't import nested namespace.
      // Example:
      //   namespace N1.N2 { class A {} }
      //   namespace N3 { using N1; class B : N2.A {} }
      // Reference |N2.A| is undefined, since |using N1| doesn't import
      // nested namespace N1.N2.
      continue;
    }
    founds->insert(present);
  }
}

sm::Semantic* NameResolver::RealNameOf(ast::Alias* alias) const {
  auto const it = alias_map_.find(alias);
  DCHECK(it != alias_map_.end());
  return it->second;
}

sm::Namespace* NameResolver::ImportedNamespaceOf(ast::Import* import) const {
  auto const it = import_map_.find(import);
  DCHECK(it != import_map_.end());
  return it->second;
}

sm::Semantic* NameResolver::ResolveReference(ast::Expression* expression,
                                             ast::ContainerNode* container) {
  ReferenceResolver resolver(this, container);
  return resolver.Resolve(expression);
}

sm::Semantic* NameResolver::SemanticOf(ast::Node* node) const {
  return analysis()->SemanticOf(node);
}

}  // namespace compiler
}  // namespace elang
