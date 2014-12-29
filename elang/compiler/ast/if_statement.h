// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_IF_STATEMENT_H_
#define ELANG_COMPILER_AST_IF_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// IfStatement
//
class IfStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(IfStatement, Statement);
  friend class NodeFactory;

 public:
  Expression* condition() const { return condition_; }
  Statement* else_statement() const { return else_statement_; }
  Statement* then_statement() const { return then_statement_; }

 private:
  IfStatement(Token* keyword,
              Expression* condition,
              Statement* then_statement,
              Statement* else_statement);
  ~IfStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const condition_;
  Statement* const else_statement_;
  Statement* const then_statement_;

  DISALLOW_COPY_AND_ASSIGN(IfStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_IF_STATEMENT_H_
