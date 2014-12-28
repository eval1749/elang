// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_unary_operation_h)
#define INCLUDE_elang_compiler_ast_unary_operation_h

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
  DECLARE_CASTABLE_CLASS(UnaryOperation, Expression);

  friend class NodeFactory;

 public:
  ~UnaryOperation() final;

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

#endif  // !defined(INCLUDE_elang_compiler_ast_unary_operation_h)
