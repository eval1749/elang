// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_ASSIGNMENT_H_
#define ELANG_COMPILER_AST_ASSIGNMENT_H_

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Assignment
//
class Assignment final : public Expression {
  DECLARE_AST_NODE_CLASS(Assignment, Expression);

 public:
  Expression* left() const { return left_; }
  Expression* right() const { return right_; }

 private:
  Assignment(Token* op, Expression* left, Expression* right);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const left_;
  Expression* const right_;

  DISALLOW_COPY_AND_ASSIGN(Assignment);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ASSIGNMENT_H_
