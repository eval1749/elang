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
// |NamespaceAnalyzer::ResolveNameReference()|.
void NameResolver::ReferenceResolver::VisitNameReference(
    ast::NameReference* node) {
  auto const name = node->name();
  if (name->is_type_name()) {
    // type keyword is mapped into |System.XXX|.
    auto const ast_class = session()->system_namespace()->FindMember(
        session()->PredefinedNameOf(name->mapped_type_name()));
    if (!ast_class)
      Error(ErrorCode::NameResolutionNameNotFound, node);
    ProduceResult(SemanticOf(ast_class));
    return;
  }

  for (ast::Node* runner = container_; runner; runner = runner->parent()) {
    auto const container = runner->is<ast::ClassBody>()
                               ? runner->as<ast::ClassBody>()->owner()
                               : runner->as<ast::ContainerNode>();
    if (!container)
      continue;

    std::unordered_set<sm::Semantic*> founds;

    if (auto const clazz = container->as<ast::Class>()) {
      FindInClass(name, SemanticOf(clazz)->as<sm::Class>(), &founds);

    } else if (auto const ns_body = container->as<ast::NamespaceBody>()) {
      // Find in enclosing namespace
      if (auto const present = ns_body->owner()->FindMember(name))
        founds.insert(SemanticOf(present));

      // Find alias
      if (auto const alias = ns_body->FindAlias(name)) {
        if (auto const present = resolver()->GetUsingReference(alias))
          founds.insert(SemanticOf(present));
      }

      if (!ns_body->FindMember(name)) {
        // When |name| isn't defined in namespace body, looking in imported
        // namespaces.
        for (auto const pair : ns_body->imports()) {
          auto const imported = resolver()->GetUsingReference(pair.second);
          if (!imported) {
            DVLOG(0) << "Not found: " << *pair.second;
            continue;
          }
          if (auto const present = imported->FindMember(name)) {
            if (present && !present->is<ast::Namespace>())
              founds.insert(SemanticOf(present));
          }
        }
      }
    } else {
      DCHECK(container->is<ast::Enum>() || container->is<ast::Method>() ||
             container->is<ast::Namespace>())
          << " container=" << *container;
      // Note: |ast::Method| holds type parameters in named node map.
      if (auto const present = container->FindMember(name))
        founds.insert(SemanticOf(present));
    }

    if (founds.size() == 1) {
      ProduceResult(*founds.begin());
      return;
    }

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

void NameResolver::DidResolveUsing(ast::NamedNode* node,
                                   ast::ContainerNode* container) {
  DCHECK(node->is<ast::Alias>() || node->is<ast::Import>());
  DCHECK(container->is<ast::Class>() || container->is<ast::Namespace>());
  DCHECK(!using_map_.count(node));
  using_map_[node] = container;
}

ast::ContainerNode* NameResolver::GetUsingReference(ast::NamedNode* node) {
  DCHECK(node->is<ast::Alias>() || node->is<ast::Import>());
  auto const it = using_map_.find(node);
  return it == using_map_.end() ? nullptr : it->second;
}

sm::Semantic* NameResolver::ResolveReference(ast::Expression* expression,
                                             ast::ContainerNode* container) {
  ReferenceResolver resolver(this, container);
  return resolver.Resolve(expression);
}

sm::Semantic* NameResolver::SemanticOf(ast::NamedNode* member) const {
  return analysis()->SemanticOf(member);
}

}  // namespace compiler
}  // namespace elang
