// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/const_expr_analyzer.h"

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
#include "elang/compiler/semantics/editor.h"
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
// ConstExprAnalyzer
//
ConstExprAnalyzer::ConstExprAnalyzer(NameResolver* name_resolver,
                                     sm::Editor* editor)
    : Analyzer(name_resolver),
      calculator_(new sm::Calculator(name_resolver->session())),
      context_(nullptr),
      editor_(editor),
      result_(nullptr),
      state_(State::Running) {
}

void ConstExprAnalyzer::AddDependency(ast::Node* from, ast::Node* to) {
  DCHECK(state_ == State::Running);
  dependency_graph_.AddEdge(from, to);
}

void ConstExprAnalyzer::AnalyzeEnumMember(ast::EnumMember* node) {
  auto const expression = ExpressionOf(node);
  auto const value = Evaluate(node, expression);
  if (!value)
    return;
  auto const member = SemanticOf(node)->as<sm::EnumMember>();
  if (value->is<sm::InvalidValue>())
    return editor_->FixEnumMember(member, value);
  auto const enum_type = member->owner();
  auto const enum_base = enum_type->enum_base();
  auto const adjusted_value = calculator()->CastAs(value, enum_base);
  if (adjusted_value->is<sm::InvalidValue>())
    Error(ErrorCode::AnalyzeExpressionType, expression, enum_base->name());
  editor_->FixEnumMember(member, adjusted_value);
}

sm::Value* ConstExprAnalyzer::Evaluate(ast::Node* context,
                                       ast::Expression* expression) {
  DCHECK(state_ != State::Finalized);
  DCHECK(!context_) << context_;
  context_ = context;
  calculator_->SetContext(context_->name());
  auto const value = Evaluate(expression);
  DCHECK_EQ(context_, context);
  context_ = nullptr;
  return value;
}

sm::Value* ConstExprAnalyzer::Evaluate(ast::Node* node) {
  DCHECK(context_);
  DCHECK(!result_) << result_;
  Traverse(node);
  DCHECK(result_ || session()->HasError() ||
         dependency_graph_.HasOutEdge(context_))
      << node << " in " << context_;
  auto const value = result_;
  result_ = nullptr;
  return value;
}

ast::Expression* ConstExprAnalyzer::ExpressionOf(ast::EnumMember* node) {
  if (auto const expression = node->expression())
    return expression;
  return node->implicit_expression();
}

void ConstExprAnalyzer::ProcessReference(ast::Expression* node) {
  auto const semantic =
      name_resolver()->ResolveReference(node, ContainerOf(node));
  if (auto const enum_member = semantic->as<sm::EnumMember>()) {
    if (enum_member->has_value())
      return ProduceResult(enum_member->value());
    if (state_ == State::Running)
      return AddDependency(context_, node);
    DCHECK(state_ == State::Finalizing);
    Error(ErrorCode::AnalyzeExpressionCycle, context_, node);
    return;
  }
  Error(ErrorCode::AnalyzeExpressionNotConstant, node);
}

void ConstExprAnalyzer::ProduceResult(sm::Value* value) {
  DCHECK(context_);
  DCHECK(!result_) << result_;
  result_ = value;
}

void ConstExprAnalyzer::Run() {
  state_ = State::Finalizing;
  std::vector<ast::Node*> leaf_nodes;
  for (auto const node : dependency_graph_.GetAllVertices()) {
    if (dependency_graph_.HasInEdge(node))
      continue;
    leaf_nodes.push_back(node);
  }
  for (auto const leaf_node : leaf_nodes) {
    for (auto const node : dependency_graph_.PostOrderListOf(leaf_node)) {
      if (auto const ast_member = node->as<ast::EnumMember>())
        AnalyzeEnumMember(ast_member);
    }
  }
  state_ = State::Finalized;
}

sm::Type* ConstExprAnalyzer::TypeFromToken(Token* token) {
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
void ConstExprAnalyzer::DoDefaultVisit(ast::Node* node) {
  if (state_ != State::Running)
    return;
  Error(ErrorCode::AnalyzeExpressionNotConstant, node);
}

void ConstExprAnalyzer::VisitBinaryOperation(ast::BinaryOperation* node) {
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

void ConstExprAnalyzer::VisitLiteral(ast::Literal* node) {
  auto const type = TypeFromToken(node->token());
  ProduceResult(factory()->NewLiteral(type, node->token()));
}

void ConstExprAnalyzer::VisitMemberAccess(ast::MemberAccess* node) {
  ProcessReference(node);
}

void ConstExprAnalyzer::VisitNameReference(ast::NameReference* node) {
  ProcessReference(node);
}

}  // namespace compiler
}  // namespace elang
