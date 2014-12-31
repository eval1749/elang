// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_UNARY_OPERATION_H_
#define ELANG_COMPILER_AST_UNARY_OPERATION_H_

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// UnaryOperation
//
class UnaryOperation final : public Expression {
  DECLARE_AST_NODE_CLASS(UnaryOperation, Expression);

 public:
  Expression* expression() const { return expression_; }

 private:
  UnaryOperation(Token* op, Expression* expression);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(UnaryOperation);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_UNARY_OPERATION_H_
