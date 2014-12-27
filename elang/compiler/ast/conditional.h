// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_conditional_h)
#define INCLUDE_elang_compiler_ast_conditional_h

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Conditional
//
class Conditional final : public Expression {
  DECLARE_CASTABLE_CLASS(Conditional, Expression);

  friend class NodeFactory;

  private: Expression* cond_;
  private: Expression* else_;
  private: Expression* then_;

  private: Conditional(Token* op, Expression* condition,
                       Expression* then_expression,
                       Expression* else_expression);
  public: ~Conditional() final;

  DISALLOW_COPY_AND_ASSIGN(Conditional);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_conditional_h)

