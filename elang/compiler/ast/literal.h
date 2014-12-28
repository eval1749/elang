// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_LITERAL_H_
#define ELANG_COMPILER_AST_LITERAL_H_

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

 public:
  ~Literal() final;

 private:
  explicit Literal(Token* literal);

  // Node
  void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_LITERAL_H_
