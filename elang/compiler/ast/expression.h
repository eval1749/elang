// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_expression_h)
#define INCLUDE_elang_compiler_ast_expression_h

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
  DECLARE_CASTABLE_CLASS(Expression, Node);

  friend class NodeFactory;

  protected: Expression(Token* op);
  public: ~Expression() override;

  public: Token* op() const { return token(); }

  DISALLOW_COPY_AND_ASSIGN(Expression);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_expression_h)

