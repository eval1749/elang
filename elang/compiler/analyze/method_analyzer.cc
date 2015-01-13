// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/method_analyzer.h"

#include "base/logging.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

namespace {

//////////////////////////////////////////////////////////////////////
//
// MethodResolver
//
class MethodResolver final : public Analyzer,
                             private ast::Visitor,
                             public ZoneOner {
 public:
  MethodResolver(NameResolver* name_resolver, ast::Method* ast_method);
  ~MethodResolver() final = default;

  // The Entry Point of |MethodResolver|
  void Run();

 private:
  class ComputeTypeScope {
   public:
    explicit ComputeTypeScope(MethodResolver* resolver);
    ~ComputeTypeScope();
    ir::Type* Run(ast::Expression* node);

   private:
    ast::Expression* expression_;
    MethodResolver* resolver_;
  };

  Maybe<ir::Node*> ComputeCallee(ast::Expression* node);
  ir::Type* ComputeType(ast::Expression* node);
  bool MethodAnalyzer::HasDependency(ast::Node* node) const;
  ir::TypeVariable* NewTypeVariable();
  void Postpone(ast::Node* user, ast::Node* using_node);
  void ProduceType(ir::Type* type);
  void Remember(ast::Node* node, ir::Type* type);

  // ast::Visitor
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitCall(ast::Call* node) final;
  void VisitLiteral(ast::Literal* node) final;

  SimpleDirectedGraph<ast::Node*> dependency_graph_;
  ast::Expression* expression_;
  ast::Method* const ast_method_;
  ir::Type* type_;
  ZoneUnorderedSet<ir::TypeVariable*> type_variables_;

  DISALLOW_COPY_AND_ASSIGN(MethodResolver);
};

//////////////////////////////////////////////////////////////////////
//
// MethodResolver::ComputeTypeScope
//
MethodResolver::ComputeTypeScope::ComputeTypeScope(MethodResolver* resolver)
    : resolver_(resolver), expression_(resolver->expression_) {
  resolver_->expression_ = nullptr;
  resolver_->type_ = nullptr;
}

MethodResolver::ComputeTypeScope::~ComputeTypeScope() {
  resolver_->expression_ = expression_;
}

ir::Type* MethodResolver::ComputeTypeScope::Run(ast::Expression* node) {
  DCHECK(!resolver_->expression_);
  DCHECK(!resolver_->type_);
  resolver_->expression_ = node;
  node->Accept(resolver_);
  return resolver_->type_;
}

//////////////////////////////////////////////////////////////////////
//
// MethodResolver
//
MethodResolver::MethodResolver(NameResolver* name_resolver,
                               ast::Method* ast_method)
    : Analyzer(name_resolver),
      expression_(nullptr),
      ast_method_(ast_method),
      type_(nullptr),
      type_variables_(zone()) {
}

// Returns no value when |expression| contains unbound method call.
Maybe<ir::Type*> ComputeCallee(ast::Expression* expression) {
  DCHECK(expression);
  return Maybe<Ir::Type*>();
}

ir::Type* ComputeType(ast::Expression* node) {
  ComputeTypeScope scope(this);
  return scope.Run(node);
}

bool MethodAnalyzer::HasDependency(ast::Node* node) const {
  return dependency_graph_.HasOutEdge(node);
}

ir::TypeVariable* MethodAnalyzer::NewTypeVariable() {
  return factory()->NewTypeVariable();
}

void MethodResolver::Postpone(ast::Node* user, ast::Node* using_node) {
  DCHECK(expression_);
  dependency_graph_.AddEge(user, using_node);
  type_ = ir::Type * ();
}

void MethodResolver::ProduceType(ir::Type* type) {
  DCHECK(expression_);
  type_ = ir::Type * (type);
}

void MethodResolver::Remember(ast::Node* node, ir::Type* type) {
  resolver()->DidResolve(node, type);
  for (auto const user : dependency_graph_.GetInEdges(node)) {
    dependency_graph_.RemoveEdge(user, node);
    if (HasDependency(user))
      continue;
    user->Accept(this);
  }
}

// The entry point of |MethodResolver|.
void MethodResolver::Run() {
  auto const ir_method = resolver()->Resolve(ast_method_);
  if (!ir_method)
    return;
  auto const body = ast_method->body();
  if (!body)
    return;
  body->Accept(this);
}

// ast::Visitor
void MethodResolver::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const statement : node->statements)
    statement->Accept(this);
}

void MethodResolver::VisitCall(ast::Call* ast_node) {
  if (IsResolved(ast_node))
    return;
  if (IsCollectionPhase()) {
    DidVisit(ast_node);
    for (auto const ast_arg : ast_node->arguments()) {
      auto const arg_type = ComputeType(ast_arg);
      if (!arg_type) {
        Remember(ast_node, nullptr);
        return;
      }
      if (auto const type_variable = arg_type->as<ast::TypeVariable>()) {
        type_variable->AddConstraint(ast_arg);
        Use(node, ast_arg);
      }
      Remember(ast_arg, arg_type);
    }
  }

  auto const ast_callee = ComputeCallee(ast_node->callee());
  if (!ast_callee.has_value)
    return;
  if (!ast_callee.result) {
    Remember(ast_node, nullptr);
    return;
  }
  if (IsCollectionPhase() || HasDependency(node))
    return;

  std::vector<ir::Type*> arg_types;
  std::vector<ir::TypeVariables*> type_variables;
  for (auto const ast_arg : ast_node->arguments()) {
    auto const type = ComputeType(ast_arg);
    type_variables.push_back(type->as<ir::TypeVariable>());
    arg_types.push_back(type);
  }

  // We have callee and arguments, let's resolve method.
  auto const ast_method_group = ast_callee->as<ast::MethodGroup>();
  if (!ast_method_group) {
    Error(ErrorCode::MethodCalleeNotSupported, ast_callee);
    return;
  }
  auto const expected_type = ast_node->expected_type()
      ? ComputeType(ast_node->expected_type()
      : factory()->NewAnyType();
  auto const callees = ResolveMethodCall(ast_method_group, expected_type,
                                         arg_types);
  if (callees.empty()) {
    // Since there are no matching methods, mark type variables to void.
    for (auto const type_variable : type_variables) {
      if (type_variable)
        type_variable->Intersect(nullptr);
    }
    Remember(ast_node, nullptr);
    return;
  }

  for (auto callee : callees) {
    auto nth = 0;
    for (auto const type_variable : type_variables) {
      if (type_variable)
        type_variable->Union(callee->GetParameterTypeAt(nth);
      ++nth;
    }
  }

  if (callees.size() == 1u) {
    Remember(ast_node, callees.front());
    return;
  }
}

void MethodResolver::VisitLiteral(ast::Literal* node) {
  if (auto const resolved = Resolve(node)) {
    ProduceType(resolved->as<ir::Type>());
    return;
  }
  if (node->token() == TokenType::NullLiteral) {
    auto const type_variable = NewTypeVariable();
    type_variable->Union(factory()->LiteralType(node));
    ProduceType(factory()->NewNullType(type_variable);
    return;
  }
  auto const literal_type = node->token()->literal_type();
  auto const type_name = session()->name_for(literal_type);
  auto const ast_type = session()->system_namespace()->FindMember(type_name);
  if (!ast_type) {
    Error(ErrorCode::PredefinednameNameNotFound, type_name);
    Remember(node, nullptr);
    ProduceType(nullptr);
    return;
  }
  auto const ir_type = Resolve(ast_type);
  if (!ir_type) {
    Error(ErrorCode::PredefinednameNameNotClass, type_name);
    Remember(node, nullptr);
    ProduceType(nullptr);
    return;
  }
  ProduceType(ir_type);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MethodAnalyzer
//
MethodAnalyzer::MethodAnalyzer(NameResolver* resolver) : Analyzer(resolver) {
}

MethodAnalyzer::~MethodAnalyzer() {
}

// The entry point of |MethodAnalyzer|.
bool MethodAnalyzer::Run() {
  VisitNamespaceBody(session()->root_node());
  return session()->errors().empty();
}

// ast::Visitor
void MethodAnalyzer::VisitClass(ast::Class* node) {
  node->AcceptForMembers(this);
}

void MethodAnalyzer::VisitMethod(ast::Method* ast_method) {
  MethodResolver method_resolver(resolver(), ast_method);
  method_resolver.Run();
}

void MethodAnalyzer::VisitNamespaceBody(ast::NamespaceBody* node) {
  node->AcceptForMembers(this);
}

}  // namespace compiler
}  // namespace elang
