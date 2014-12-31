// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_EXPRESSION_STATEMENT_H_
#define ELANG_COMPILER_AST_EXPRESSION_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// ExpressionStatement
//
class ExpressionStatement final : public Statement {
  DECLARE_AST_NODE_CLASS(ExpressionStatement, Statement);

 public:
  Expression* expression() const { return expression_; }

 private:
  explicit ExpressionStatement(Expression* expression);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_EXPRESSION_STATEMENT_H_
