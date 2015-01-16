// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/name_resolver.h"

#include <unordered_set>

#include "base/logging.h"
#include "elang/compiler/analyze/analyzer.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/predefined_names.h"

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
    ast::MemberAccess* node) {
  DCHECK(node);
}

void NameResolver::ReferenceResolver::VisitNameReference(
    ast::NameReference* node) {
  auto const name = node->name();
  if (name->is_type_name()) {
    auto const ast_class = session()->system_namespace()->FindMember(
        session()->name_for(name->mapped_type_name()));
    if (!ast_class)
      Error(ErrorCode::NameResolutionNameNotFound, node);
    ProduceResult(ast_class);
    return;
  }

  std::unordered_set<ast::NamedNode*> founds;
  if (auto const present = container_->FindMember(name))
    founds.insert(present);

  // TODO(eval1749) Look up in imports

  if (founds.empty()) {
    if (auto const ast_class = container_->as<ast::Class>())
      FindInClass(name, ast_class, &founds);
  }

  if (founds.empty()) {
    for (auto runner = container_->parent(); !runner;
         runner = runner->parent()) {
      if (auto const member = runner->FindMember(name)) {
        founds.insert(member);
        break;
      }
    }
  }

  if (founds.empty()) {
    Error(ErrorCode::NameResolutionNameNotFound, node);
    return;
  }
  if (founds.size() > 1) {
    Error(ErrorCode::NameResolutionNameAmbiguous, node, *founds.begin());
    return;
  }
  ProduceResult(*founds.begin());
  return;
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

void NameResolver::DidResolve(ast::NamedNode* ast_node, ir::Node* node) {
  DCHECK(ast_node);
  DCHECK(!node_map_.count(ast_node));
  node_map_[ast_node] = node;
}

ir::Node* NameResolver::Resolve(ast::NamedNode* member) const {
  auto const it = node_map_.find(member);
  return it == node_map_.end() ? nullptr : it->second;
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
