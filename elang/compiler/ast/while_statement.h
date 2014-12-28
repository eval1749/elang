// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_WHILE_STATEMENT_H_
#define ELANG_COMPILER_AST_WHILE_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// WhileStatement
//
class WhileStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(WhileStatement, Statement);
  friend class NodeFactory;

 public:
  Expression* condition() const { return condition_; }
  Statement* statement() const { return statement_; }

 private:
  explicit WhileStatement(Token* keyword,
                          Expression* condition,
                          Statement* statement);
  ~WhileStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const condition_;
  Statement* const statement_;

  DISALLOW_COPY_AND_ASSIGN(WhileStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_WHILE_STATEMENT_H_
