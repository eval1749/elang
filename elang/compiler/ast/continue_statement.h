// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CONTINUE_STATEMENT_H_
#define ELANG_COMPILER_AST_CONTINUE_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// ContinueStatement
//
class ContinueStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(ContinueStatement, Statement);
  friend class NodeFactory;

 private:
  explicit ContinueStatement(Token* keyword);
  ~ContinueStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(ContinueStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CONTINUE_STATEMENT_H_
