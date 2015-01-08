// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_EXPRESSION_H_
#define ELANG_COMPILER_AST_EXPRESSION_H_

#include <vector>

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Expression
//
class Expression : public Node {
  DECLARE_AST_NODE_CLASS(Expression, Node);

 public:
  Token* op() const { return token(); }

 protected:
  explicit Expression(Token* op);

 private:
  DISALLOW_COPY_AND_ASSIGN(Expression);
};

// Represents invalid expression. This expression is used for continuing parsing
// after syntax error.
class InvalidExpression final : public Expression {
  DECLARE_AST_NODE_CONCRETE_CLASS(InvalidExpression, Expression);

 private:
  explicit InvalidExpression(Token* token);

  DISALLOW_COPY_AND_ASSIGN(InvalidExpression);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_EXPRESSION_H_
