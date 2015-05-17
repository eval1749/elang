// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_CONST_EXPR_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_CONST_EXPR_ANALYZER_H_

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
class Editor;
class Semantic;
class Value;
}

//////////////////////////////////////////////////////////////////////
//
// ConstExprAnalyzer
//
class ConstExprAnalyzer final : public Analyzer, public ast::Visitor {
 public:
  ConstExprAnalyzer(NameResolver* name_resolver, sm::Editor* editor);
  ~ConstExprAnalyzer() = default;

  sm::Calculator* calculator() const { return calculator_.get(); }
  sm::Editor* editor() const { return editor_; }

  void AnalyzeEnumMember(ast::EnumMember* node);
  void Run();

 private:
  enum class State {
    Finalized,
    Finalizing,
    Running,
  };

  void AddDependency(ast::Node* from, ast::Node* to);
  sm::Value* Evaluate(ast::Node* context, ast::Expression* expression);
  sm::Value* Evaluate(ast::Node* node);
  ast::Expression* ExpressionOf(ast::EnumMember* node);
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
  ast::Node* context_;
  SimpleDirectedGraph<ast::Node*> dependency_graph_;
  sm::Editor* const editor_;
  sm::Value* result_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(ConstExprAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_CONST_EXPR_ANALYZER_H_
