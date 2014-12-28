// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_binary_operation_h)
#define INCLUDE_elang_compiler_ast_binary_operation_h

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// BinaryOperation
//
class BinaryOperation final : public Expression {
  DECLARE_CASTABLE_CLASS(BinaryOperation, Expression);

  friend class NodeFactory;

 public:
  ~BinaryOperation() final;

  Expression* left() const { return left_; }
  Expression* right() const { return right_; }

 private:
  BinaryOperation(Token* op, Expression* left, Expression* right);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const left_;
  Expression* const right_;

  DISALLOW_COPY_AND_ASSIGN(BinaryOperation);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_binary_operation_h)
