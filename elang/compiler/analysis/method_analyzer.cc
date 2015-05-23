// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/method_analyzer.h"

#include "base/logging.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/type_resolver.h"
#include "elang/compiler/analysis/type_factory.h"
#include "elang/compiler/analysis/type_values.h"
#include "elang/compiler/analysis/variable_tracker.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
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
  ts::Value* empty_value() const { return type_factory()->empty_value(); }
  ts::Factory* type_factory() const { return type_factory_.get(); }
  TypeResolver* type_resolver() const { return type_resolver_.get(); }
  sm::Type* void_type() const;

  ts::Value* Analyze(ast::Expression* expressions, ts::Value* value);
  void Analyze(ast::Statement* statement);
  void AnalyzeAsBool(ast::Expression* expression);
  ts::Value* AnalyzeVariable(ast::Variable* variable, ts::Value* super);
  ts::Value* NewLiteral(sm::Type* type);
  void RegisterParameters();

  // ast::Visitor
  void DoDefaultVisit(ast::Node* node) final;
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitBreakStatement(ast::BreakStatement* node) final;
  void VisitContinueStatement(ast::ContinueStatement* node) final;
  void VisitDoStatement(ast::DoStatement* node) final;
  void VisitEmptyStatement(ast::EmptyStatement* node) final;
  void VisitExpressionList(ast::ExpressionList* node) final;
  void VisitExpressionStatement(ast::ExpressionStatement* node) final;
  void VisitForEachStatement(ast::ForEachStatement* node) final;
  void VisitForStatement(ast::ForStatement* node) final;
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

sm::Type* MethodBodyAnalyzer::void_type() const {
  return session()->PredefinedTypeOf(PredefinedName::Void);
}

ts::Value* MethodBodyAnalyzer::Analyze(ast::Expression* expression,
                                       ts::Value* value) {
  return type_resolver()->Resolve(expression, value);
}

void MethodBodyAnalyzer::Analyze(ast::Statement* statement) {
  if (!statement)
    return;
  Traverse(statement);
}

void MethodBodyAnalyzer::AnalyzeAsBool(ast::Expression* expression) {
  type_resolver()->ResolveAsBool(expression);
}

ts::Value* MethodBodyAnalyzer::AnalyzeVariable(ast::Variable* variable,
                                               ts::Value* super) {
  if (variable->type()->is<ast::TypeVariable>()) {
    // Assign type variable for variable declared with `var`.
    auto const type_variable = type_factory()->NewVariable(variable, super);
    variable_tracker_->RegisterVariable(variable, type_variable);
    return type_variable;
  }
  auto const type = ResolveTypeReference(variable->type(), method_);
  if (!type) {
    variable_tracker_->RegisterVariable(variable, empty_value());
    return nullptr;
  }
  auto const var_value = NewLiteral(type);
  if (type_resolver()->Unify(var_value, super) == empty_value())
    Error(ErrorCode::TypeResolverForEachElementType, variable);
  variable_tracker_->RegisterVariable(variable, var_value);
  return var_value;
}

ts::Value* MethodBodyAnalyzer::NewLiteral(sm::Type* type) {
  return type_factory()->NewLiteral(type);
}

void MethodBodyAnalyzer::RegisterParameters() {
  for (auto const parameter : method_->parameters()) {
    auto const type = ResolveTypeReference(parameter->type(), method_);
    auto const value = NewLiteral(type);
    variable_tracker_->RegisterVariable(parameter, value);
  }
}

// The entry point of |MethodBodyAnalyzer|.
void MethodBodyAnalyzer::Run() {
  auto const ir_method = analysis()->SemanticOf(method_)->as<sm::Method>();
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

  if (auto const expression = body->as<ast::Expression>()) {
    Analyze(expression, NewLiteral(ir_method->return_type()));
  } else if (auto const statement = body->as<ast::Statement>()) {
    Analyze(body);
  } else if (!body) {
    NOTREACHED() << "Unexpected body node: " << *body;
  }

  for (auto const call_value : type_resolver_->call_values()) {
    auto const call = call_value->ast_call();
    auto const methods = call_value->methods();
    if (methods.empty()) {
      Error(ErrorCode::TypeResolverMethodNoMatch, call);
      continue;
    }
    if (methods.size() == 1u) {
      SetSemanticOf(call->callee(), methods.front());
      continue;
    }
    Error(ErrorCode::TypeResolverMethodAmbiguous, call);
  }

  variable_tracker_->Finish(type_factory());
}

// ast::Visitor statements
void MethodBodyAnalyzer::DoDefaultVisit(ast::Node* node) {
  Error(ErrorCode::TypeResolverStatementNotYetImplemented, node);
}

void MethodBodyAnalyzer::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const statement : node->statements()) {
    Analyze(statement);
    if (statement->IsTerminator()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
  }
}

void MethodBodyAnalyzer::VisitBreakStatement(ast::BreakStatement* node) {
  DCHECK(node);
}

void MethodBodyAnalyzer::VisitContinueStatement(ast::ContinueStatement* node) {
  DCHECK(node);
}

void MethodBodyAnalyzer::VisitDoStatement(ast::DoStatement* node) {
  Analyze(node->statement());
  AnalyzeAsBool(node->condition());
}

void MethodBodyAnalyzer::VisitEmptyStatement(ast::EmptyStatement* node) {
  DCHECK(node);
}

void MethodBodyAnalyzer::VisitExpressionList(ast::ExpressionList* node) {
  for (auto const expression : node->expressions())
    Analyze(expression, type_factory()->any_value());
}

void MethodBodyAnalyzer::VisitExpressionStatement(
    ast::ExpressionStatement* node) {
  Analyze(node->expression(), type_factory()->any_value());
}

// A 'expression' of for-each statement can be one of
//  - |System.Array|
//  - |System.Collections.IEnumerable<T>|
//  - Type X which has |GetEnumerator() -> E|, |E.Current() -> T|,
//    |E.MoveNext() -> System.Bool|.
//
// TODO(eval1749) We should support |IEnumerable<T>| and |GetEnumerator()|.
void MethodBodyAnalyzer::VisitForEachStatement(ast::ForEachStatement* node) {
  auto const type = Analyze(node->enumerable(), type_factory()->any_value());
  auto const literal = type->as<ts::Literal>();
  auto const variable = node->variable();
  if (!literal) {
    Error(ErrorCode::TypeResolverStatementNotYetImplemented, node);
    return;
  }
  auto const array_type = literal->value()->as<sm::ArrayType>();
  if (!array_type) {
    Error(ErrorCode::TypeResolverStatementNotYetImplemented, node);
    return;
  }
  auto const element_type = NewLiteral(array_type->element_type());
  AnalyzeVariable(variable, element_type);
  Analyze(node->statement());
}

void MethodBodyAnalyzer::VisitForStatement(ast::ForStatement* node) {
  Analyze(node->initializer());
  AnalyzeAsBool(node->condition());
  Analyze(node->step());
  Analyze(node->statement());
}

void MethodBodyAnalyzer::VisitIfStatement(ast::IfStatement* node) {
  AnalyzeAsBool(node->condition());
  Analyze(node->then_statement());
  Analyze(node->else_statement());
}

void MethodBodyAnalyzer::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const ir_method = analysis()->SemanticOf(method_)->as<sm::Method>();
  auto const return_type = ir_method->return_type();
  if (ir_method->return_type() == void_type()) {
    if (node->value())
      Error(ErrorCode::MethodReturnNotVoid, node);
    return;
  }
  if (auto const return_value = node->value()) {
    Analyze(return_value, NewLiteral(return_type));
    return;
  }
  Error(ErrorCode::MethodReturnVoid, node);
}

void MethodBodyAnalyzer::VisitVarStatement(ast::VarStatement* node) {
  for (auto const var_decl : node->variables()) {
    auto const variable_type =
        AnalyzeVariable(var_decl->variable(), type_factory()->any_value());
    if (!variable_type)
      continue;
    // Check initial value expression matches variable type.
    Analyze(var_decl->value(), variable_type);
  }
}

void MethodBodyAnalyzer::VisitWhileStatement(ast::WhileStatement* node) {
  AnalyzeAsBool(node->condition());
  Analyze(node->statement());
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
void MethodAnalyzer::Run() {
  Traverse(session()->global_namespace_body());
}

// ast::Visitor
void MethodAnalyzer::VisitMethod(ast::Method* method) {
  MethodBodyAnalyzer method_resolver(resolver(), method);
  method_resolver.Run();
}

}  // namespace compiler
}  // namespace elang
