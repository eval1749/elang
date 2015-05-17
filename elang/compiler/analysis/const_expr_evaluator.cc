// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/const_expr_evaluator.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/calculator.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {
ast::ContainerNode* ContainerOf(ast::Node* node) {
  for (auto runner = node; runner; runner = runner->parent()) {
    if (auto const container = runner->as<ast::ContainerNode>())
      return container;
  }
  NOTREACHED() << node;
  return nullptr;
}
}

//////////////////////////////////////////////////////////////////////
//
// ConstExprEvaluator
//
ConstExprEvaluator::ConstExprEvaluator(NameResolver* name_resolver)
    : Analyzer(name_resolver),
      calculator_(new sm::Calculator(name_resolver->session())),
      context_(nullptr),
      result_(nullptr) {
}

void ConstExprEvaluator::AddDependency(sm::Semantic* from, sm::Semantic* to) {
  dependency_graph_.AddEdge(from, to);
}

sm::Value* ConstExprEvaluator::Evaluate(sm::Semantic* context,
                                        ast::Node* expression) {
  DCHECK(!context_) << context_;
  context_ = context;
  auto const value = Evaluate(expression);
  DCHECK(context_);
  context_ = nullptr;
  return value;
}

sm::Value* ConstExprEvaluator::Evaluate(ast::Node* node) {
  DCHECK(context_);
  DCHECK(!result_) << result_;
  Traverse(node);
  DCHECK(result_ || session()->HasError());
  auto const value = result_;
  result_ = nullptr;
  return value;
}

void ConstExprEvaluator::ProcessReference(ast::Expression* node) {
  auto const semantic =
      name_resolver()->ResolveReference(node, ContainerOf(node));
  if (auto const enum_member = semantic->as<sm::EnumMember>()) {
    if (enum_member->has_value())
      return ProduceResult(enum_member->value());
    return AddDependency(context_, enum_member);
  }
  Error(ErrorCode::AnalyzeExpressionNotConstant, node);
}

void ConstExprEvaluator::ProduceResult(sm::Value* value) {
  DCHECK(context_);
  DCHECK(!result_) << result_;
  result_ = value;
}

sm::Type* ConstExprEvaluator::TypeFromToken(Token* token) {
  switch (token->type()) {
    case TokenType::CharacterLiteral:
      return session()->PredefinedTypeOf(PredefinedName::Char);
    case TokenType::FalseLiteral:
      return session()->PredefinedTypeOf(PredefinedName::Bool);
    case TokenType::Float32Literal:
      return session()->PredefinedTypeOf(PredefinedName::Float32);
    case TokenType::Float64Literal:
      return session()->PredefinedTypeOf(PredefinedName::Float64);
    case TokenType::Int32Literal:
      return session()->PredefinedTypeOf(PredefinedName::Int32);
    case TokenType::Int64Literal:
      return session()->PredefinedTypeOf(PredefinedName::Int64);
    case TokenType::StringLiteral:
      return session()->PredefinedTypeOf(PredefinedName::String);
    case TokenType::TrueLiteral:
      return session()->PredefinedTypeOf(PredefinedName::Bool);
    case TokenType::UInt32Literal:
      return session()->PredefinedTypeOf(PredefinedName::UInt32);
    case TokenType::UInt64Literal:
      return session()->PredefinedTypeOf(PredefinedName::UInt64);
  }
  NOTREACHED() << token;
  return nullptr;
}

// ast::Visitor
void ConstExprEvaluator::DoDefaultVisit(ast::Node* node) {
  Error(ErrorCode::AnalyzeExpressionNotConstant, node);
}

void ConstExprEvaluator::VisitBinaryOperation(ast::BinaryOperation* node) {
  auto const left = Evaluate(node->left());
  if (!left)
    return;
  auto const right = Evaluate(node->right());
  if (!right)
    return;
  switch (node->token()->type()) {
    case TokenType::Add:
      ProduceResult(calculator_->Add(left, right));
      return;
  }
  NOTREACHED();
}

void ConstExprEvaluator::VisitLiteral(ast::Literal* node) {
  auto const type = TypeFromToken(node->token());
  ProduceResult(factory()->NewLiteral(type, node->token()));
}

void ConstExprEvaluator::VisitMemberAccess(ast::MemberAccess* node) {
  ProcessReference(node);
}

void ConstExprEvaluator::VisitNameReference(ast::NameReference* node) {
  ProcessReference(node);
}

}  // namespace compiler
}  // namespace elang
