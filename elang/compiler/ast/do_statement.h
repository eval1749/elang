// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_DO_STATEMENT_H_
#define ELANG_COMPILER_AST_DO_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// DoStatement
//
class DoStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(DoStatement, Statement);
  friend class NodeFactory;

 public:
  Expression* condition() const { return condition_; }
  Statement* statement() const { return statement_; }

 private:
  DoStatement(Token* keyword, Statement* statement, Expression* condition);
  ~DoStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const condition_;
  Statement* const statement_;

  DISALLOW_COPY_AND_ASSIGN(DoStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_DO_STATEMENT_H_
