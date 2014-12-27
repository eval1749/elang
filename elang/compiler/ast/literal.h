// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_literal_h)
#define INCLUDE_elang_compiler_ast_literal_h

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Literal
//
class Literal final : public Expression {
  DECLARE_CASTABLE_CLASS(Literal, Expression);

  friend class NodeFactory;

  private: Literal(Token* literal);
  public: ~Literal() final;

  // Node
  private: void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_literal_h)

