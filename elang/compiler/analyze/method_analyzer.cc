// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/method_analyzer.h"

#include "base/logging.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/analyze/type_resolver.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/analyze/variable_tracker.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {

//////////////////////////////////////////////////////////////////////
//
// MethodBodyAnalyzer
//
class MethodBodyAnalyzer final : public Analyzer,
                                 public ZoneOwner,
                                 private ast::Visitor {
 public:
  MethodBodyAnalyzer(NameResolver* name_resolver, ast::Method* method);
  ~MethodBodyAnalyzer() final = default;

  // The Entry Point of |MethodBodyAnalyzer|
  void Run();

 private:
  ts::Factory* type_factory() const { return type_factory_.get(); }
  TypeResolver* type_resolver() const { return type_resolver_.get(); }
  ir::Type* void_type() const;

  void RegisterParameters();

  // ast::Visitor
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitDoStatement(ast::DoStatement* node) final;
  void VisitExpressionStatement(ast::ExpressionStatement* node) final;
  void VisitIfStatement(ast::IfStatement* node) final;
  void VisitReturnStatement(ast::ReturnStatement* node) final;
  void VisitVarStatement(ast::VarStatement* node) final;
  void VisitWhileStatement(ast::WhileStatement* node) final;

  // Owner of method body.
  ast::Method* const method_;
  const std::unique_ptr<ts::Factory> type_factory_;
  const std::unique_ptr<VariableTracker> variable_tracker_;

  // |TypeResolver| should be constructed after |VariableTracker|.
  const std::unique_ptr<TypeResolver> type_resolver_;

  DISALLOW_COPY_AND_ASSIGN(MethodBodyAnalyzer);
};

//////////////////////////////////////////////////////////////////////
//
// MethodBodyAnalyzer
// Traversal statements in method body.
//
MethodBodyAnalyzer::MethodBodyAnalyzer(NameResolver* name_resolver,
                                       ast::Method* method)
    : Analyzer(name_resolver),
      method_(method),
      type_factory_(new ts::Factory(session(), zone())),
      variable_tracker_(new VariableTracker(session(), zone(), method)),
      type_resolver_(new TypeResolver(name_resolver,
                                      type_factory_.get(),
                                      variable_tracker_.get(),
                                      method)) {
}

ir::Type* MethodBodyAnalyzer::void_type() const {
  return semantics()
      ->ValueOf(session()->GetPredefinedType(PredefinedName::Void))
      ->as<ir::Type>();
}

void MethodBodyAnalyzer::RegisterParameters() {
  for (auto const parameter : method_->parameters()) {
    auto const type = ResolveTypeReference(parameter->type(), method_);
    auto const value = type_resolver()->type_factory()->NewLiteral(type);
    variable_tracker_->RegisterVariable(parameter, value);
  }
}

// The entry point of |MethodBodyAnalyzer|.
void MethodBodyAnalyzer::Run() {
  auto const ir_method = semantics()->ValueOf(method_);
  if (!ir_method) {
    DVLOG(0) << *method_ << " isn't resolved.";
    return;
  }
  auto const body = method_->body();
  if (!body) {
    DCHECK(method_->IsExtern() || method_->IsAbstract())
        << *method_ << " should have a body.";
    return;
  }
  DCHECK(!method_->IsExtern() && !method_->IsAbstract())
      << *method_ << " should not have a body.";
  RegisterParameters();
  body->Accept(this);
  for (auto const call_value : type_resolver_->call_values()) {
    auto const call = call_value->ast_call();
    auto const methods = call_value->methods();
    if (methods.empty()) {
      Error(ErrorCode::TypeResolverMethodNoMatch, call);
      continue;
    }
    if (methods.size() == 1u) {
      semantics()->SetValue(call->callee(), methods.front());
      continue;
    }
    Error(ErrorCode::TypeResolverMethodAmbiguous, call);
  }
  variable_tracker_->Finish(factory(), type_factory());
}

// ast::Visitor statements
void MethodBodyAnalyzer::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const statement : node->statements()) {
    statement->Accept(this);
    if (statement->IsTerminator()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
  }
}

void MethodBodyAnalyzer::VisitDoStatement(ast::DoStatement* node) {
  node->statement()->Accept(this);
  type_resolver()->ResolveAsBool(node->condition());
}

void MethodBodyAnalyzer::VisitExpressionStatement(
    ast::ExpressionStatement* node) {
  type_resolver()->Resolve(node->expression(), type_factory()->any_value());
}

void MethodBodyAnalyzer::VisitIfStatement(ast::IfStatement* node) {
  type_resolver()->ResolveAsBool(node->condition());
  node->then_statement()->Accept(this);
  if (!node->else_statement())
    return;
  node->else_statement()->Accept(this);
}

void MethodBodyAnalyzer::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const ir_method = semantics()->ValueOf(method_)->as<ir::Method>();
  auto const return_type = ir_method->return_type();
  if (ir_method->return_type() == void_type()) {
    if (node->value())
      Error(ErrorCode::MethodReturnNotVoid, node);
    return;
  }
  if (auto const return_value = node->value()) {
    auto const value = type_factory()->NewLiteral(return_type);
    type_resolver()->Resolve(return_value, value);
    return;
  }
  Error(ErrorCode::MethodReturnVoid, node);
}

void MethodBodyAnalyzer::VisitVarStatement(ast::VarStatement* node) {
  for (auto const variable : node->variables()) {
    if (!variable->value())
      continue;
    if (auto const reference = variable->type()) {
      if (reference->name() == TokenType::Var) {
        // Assign type variable for variable declared with `var`.
        auto const type_variable = type_resolver()->type_factory()->NewVariable(
            variable, type_resolver()->type_factory()->any_value());
        variable_tracker_->RegisterVariable(variable, type_variable);
        type_resolver()->Resolve(variable->value(), type_variable);
        continue;
      }
    }
    auto const type = ResolveTypeReference(variable->type(), method_);
    if (!type)
      continue;
    auto const value = type_resolver()->type_factory()->NewLiteral(type);
    variable_tracker_->RegisterVariable(variable, value);
    // Check initial value expression matches variable type.
    type_resolver()->Resolve(variable->value(), value);
  }
}

void MethodBodyAnalyzer::VisitWhileStatement(ast::WhileStatement* node) {
  type_resolver()->ResolveAsBool(node->condition());
  node->statement()->Accept(this);
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
  VisitNamespaceBody(session()->global_namespace_body());
  return session()->errors().empty();
}

// ast::Visitor
void MethodAnalyzer::VisitMethod(ast::Method* method) {
  MethodBodyAnalyzer method_resolver(resolver(), method);
  method_resolver.Run();
}

}  // namespace compiler
}  // namespace elang
