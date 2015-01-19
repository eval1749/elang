// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/name_resolver.h"

#include <unordered_set>

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/analyze/analyzer.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics.h"

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
  ~ReferenceResolver() = default;

  ast::NamedNode* Resolve(ast::Expression* expression);

 private:
  void FindInClass(Token* name,
                   ast::Class* ast_class,
                   std::unordered_set<ast::NamedNode*>* founds);
  void ProduceResult(ast::NamedNode* result);

  // ast::Visitor
  void VisitMemberAccess(ast::MemberAccess* node) final;
  void VisitNameReference(ast::NameReference* node) final;
  void VisitTypeMemberAccess(ast::TypeMemberAccess* node) final;
  void VisitTypeNameReference(ast::TypeNameReference* node) final;

  ast::ContainerNode* const container_;
  ast::NamedNode* result_;

  DISALLOW_COPY_AND_ASSIGN(ReferenceResolver);
};

NameResolver::ReferenceResolver::ReferenceResolver(
    NameResolver* name_resolver,
    ast::ContainerNode* container)
    : Analyzer(name_resolver), container_(container), result_(nullptr) {
}

void NameResolver::ReferenceResolver::FindInClass(
    Token* name,
    ast::Class* ast_class,
    std::unordered_set<ast::NamedNode*>* founds) {
  if (auto const present = ast_class->FindMember(name)) {
    founds->insert(present);
    return;
  }
  auto const ir_node = resolver()->Resolve(ast_class);
  if (!ir_node) {
    Error(ErrorCode::NameResolutionNameNotResolved, ast_class);
    return;
  }
  auto const ir_class = ir_node->as<ir::Class>();
  if (!ir_class) {
    Error(ErrorCode::NameResolutionNameNotResolved, ast_class);
    return;
  }
  for (auto const ir_base_class : ir_class->base_classes())
    FindInClass(name, ir_base_class->ast_class(), founds);
}

void NameResolver::ReferenceResolver::ProduceResult(ast::NamedNode* result) {
  DCHECK(!result_);
  result_ = result;
}

ast::NamedNode* NameResolver::ReferenceResolver::Resolve(
    ast::Expression* expression) {
  DCHECK(!result_);
  expression->Accept(this);
  return result_;
}

// ast::Visitor
void NameResolver::ReferenceResolver::VisitMemberAccess(
    ast::MemberAccess* reference) {
  auto resolved = static_cast<ast::NamedNode*>(nullptr);
  auto container = container_;
  for (auto const component : reference->components()) {
    if (resolved) {
      container = resolved->as<ast::ContainerNode>();
      if (!container) {
        Error(ErrorCode::NameResolutionNameNeitherNamespaceNorType, component,
              reference);
        ProduceResult(nullptr);
        return;
      }
    }

    resolved = resolver()->ResolveReference(component, container);
    if (!resolved) {
      ProduceResult(nullptr);
      return;
    }
  }
  DCHECK(resolved);
  ProduceResult(resolved);
}

// Algorithm of this function should be equivalent to
// |NamespaceAnalyzer::ResolveNameReference()|.
void NameResolver::ReferenceResolver::VisitNameReference(
    ast::NameReference* node) {
  auto const name = node->name();
  if (name->is_type_name()) {
    // type keyword is mapped into |System.XXX|.
    auto const ast_class = session()->system_namespace()->FindMember(
        session()->name_for(name->mapped_type_name()));
    if (!ast_class)
      Error(ErrorCode::NameResolutionNameNotFound, node);
    ProduceResult(ast_class);
    return;
  }

  for (auto runner = container_; runner; runner = runner->parent()) {
    auto const container = runner->is<ast::ClassBody>()
                               ? runner->as<ast::ClassBody>()->owner()
                               : runner;

    std::unordered_set<ast::NamedNode*> founds;

    if (auto const clazz = container->as<ast::Class>()) {
      if (auto const present = clazz->FindMember(name))
        FindInClass(name, clazz, &founds);

    } else if (auto const ns_body = container->as<ast::NamespaceBody>()) {
      // Find in enclosing namespace
      if (auto const present = ns_body->owner()->FindMember(name))
        founds.insert(present);

      // Find alias
      if (auto const alias = ns_body->FindAlias(name)) {
        if (auto const present = resolver()->GetUsingReference(alias))
          founds.insert(present);
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
              founds.insert(present);
          }
        }
      }
    } else {
      DCHECK(container->is<ast::Method>() || container->is<ast::Namespace>())
          << " container=" << *container;
      // Note: |ast::Method| holds type parameters in named node map.
      if (auto const present = container->FindMember(name))
        founds.insert(present);
    }

    if (founds.size() == 1) {
      ProduceResult(*founds.begin());
      return;
    }

    if (founds.size() > 1) {
      Error(ErrorCode::NameResolutionNameAmbiguous, node, *founds.begin());
      return;
    }
  }

  DVLOG(0) << "Not found " << name << " in " << *container_ << std::endl;
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
    : factory_(new ir::Factory()), session_(session) {
}

NameResolver::~NameResolver() {
}

Semantics* NameResolver::semantics() const {
  return session()->semantics();
}

void NameResolver::DidResolve(ast::NamedNode* ast_node, ir::Node* node) {
  DCHECK(ast_node);
  DCHECK(!semantics()->ValueOf(ast_node));
  semantics()->SetValue(ast_node, node);
}

void NameResolver::DidResolveCall(ast::Call* ast_call, ir::Method* method) {
  DCHECK(ast_call);
  DCHECK(!semantics()->MethodOf(ast_call));
  semantics()->SetMethod(ast_call, method);
}

void NameResolver::DidResolveUsing(ast::NamedNode* node,
                                   ast::ContainerNode* container) {
  DCHECK(node->is<ast::Alias>() || node->is<ast::Import>());
  DCHECK(container->is<ast::Class>() || container->is<ast::Namespace>());
  DCHECK(!using_map_.count(node));
  using_map_[node] = container;
}

ir::Type* NameResolver::GetPredefinedType(PredefinedName name) {
  auto const type_name = session()->name_for(name);
  auto const ast_type = session()->system_namespace()->FindMember(type_name);
  DCHECK(ast_type) << *type_name;
  auto const type = Resolve(ast_type)->as<ir::Type>();
  DCHECK(type) << *type_name;
  return type;
  return nullptr;
}

ast::ContainerNode* NameResolver::GetUsingReference(ast::NamedNode* node) {
  DCHECK(node->is<ast::Alias>() || node->is<ast::Import>());
  auto const it = using_map_.find(node);
  return it == using_map_.end() ? nullptr : it->second;
}

ir::Node* NameResolver::Resolve(ast::NamedNode* member) const {
  return semantics()->ValueOf(member);
}

ir::Method* NameResolver::ResolveCall(ast::Call* ast_call) const {
  return semantics()->MethodOf(ast_call);
}

ir::Type* NameResolver::ResolvePredefinedType(Token* token,
                                              PredefinedName name) {
  auto const type_name = session()->name_for(name);
  auto const ast_type = session()->system_namespace()->FindMember(type_name);
  if (!ast_type) {
    session()->AddError(ErrorCode::PredefinedNamesNameNotFound, token);
    return nullptr;
  }
  if (auto const type = Resolve(ast_type)->as<ir::Type>())
    return type;
  session()->AddError(ErrorCode::PredefinedNamesNameNotClass, token);
  DidResolve(ast_type, nullptr);
  return nullptr;
}

ast::NamedNode* NameResolver::ResolveReference(ast::Expression* expression,
                                               ast::ContainerNode* container) {
  ReferenceResolver resolver(this, container);
  return resolver.Resolve(expression);
}

}  // namespace compiler
}  // namespace elang
