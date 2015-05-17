// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_CONST_EXPR_EVALUATOR_H_
#define ELANG_COMPILER_ANALYSIS_CONST_EXPR_EVALUATOR_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {
class NameResolver;
namespace sm {
class Calculator;
class Semantic;
class Value;
}

//////////////////////////////////////////////////////////////////////
//
// ConstExprEvaluator
//
class ConstExprEvaluator final : public Analyzer, public ast::Visitor {
 public:
  explicit ConstExprEvaluator(NameResolver* name_resolver);
  ~ConstExprEvaluator() = default;

  sm::Calculator& calculator() const { return *calculator_; }

  void AddDependency(sm::Semantic* from, sm::Semantic* to);
  sm::Value* Evaluate(sm::Semantic* context, ast::Node* expression);

 private:
  sm::Value* Evaluate(ast::Node* node);
  void ProcessReference(ast::Expression* node);
  void ProduceResult(sm::Value* value);
  sm::Type* TypeFromToken(Token* token);

  // ast::Visitor
  void DoDefaultVisit(ast::Node* node) final;
  void VisitBinaryOperation(ast::BinaryOperation* node) final;
  void VisitLiteral(ast::Literal* node) final;
  void VisitMemberAccess(ast::MemberAccess* node) final;
  void VisitNameReference(ast::NameReference* node) final;

  const std::unique_ptr<sm::Calculator> calculator_;
  sm::Semantic* context_;
  SimpleDirectedGraph<sm::Semantic*> dependency_graph_;
  sm::Value* result_;

  DISALLOW_COPY_AND_ASSIGN(ConstExprEvaluator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_CONST_EXPR_EVALUATOR_H_
