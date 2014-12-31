// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CONDITIONAL_H_
#define ELANG_COMPILER_AST_CONDITIONAL_H_

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Conditional
//
class Conditional final : public Expression {
  DECLARE_AST_NODE_CLASS(Conditional, Expression);

 public:
  Expression* conditional() const { return cond_; }
  Expression* else_expression() const { return else_; }
  Expression* then_expression() const { return then_; }

 private:
  Conditional(Token* op,
              Expression* condition,
              Expression* then_expression,
              Expression* else_expression);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const cond_;
  Expression* const else_;
  Expression* const then_;

  DISALLOW_COPY_AND_ASSIGN(Conditional);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CONDITIONAL_H_
