// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_assignment_h)
#define INCLUDE_elang_compiler_ast_assignment_h

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Assignment
//
class Assignment final : public Expression {
  DECLARE_CASTABLE_CLASS(Assignment, Expression);

  friend class NodeFactory;

  private: Expression* const left_;
  private: Expression* const right_;

  private: Assignment(Token* op, Expression* left, Expression* right);
  public: ~Assignment() final;

  public: Expression* left() const { return left_; }
  public: Expression* right() const { return right_; }

  // Node
  private: void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(Assignment);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_assignment_h)

