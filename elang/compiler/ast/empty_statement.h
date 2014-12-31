// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_EMPTY_STATEMENT_H_
#define ELANG_COMPILER_AST_EMPTY_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// EmptyStatement
//
class EmptyStatement final : public Statement {
  DECLARE_AST_NODE_CLASS(EmptyStatement, Statement);

 private:
  explicit EmptyStatement(Token* keyword);

  // Node
  void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(EmptyStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_EMPTY_STATEMENT_H_
