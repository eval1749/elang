// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_BLOCK_STATEMENT_H_
#define ELANG_COMPILER_AST_BLOCK_STATEMENT_H_

#include <vector>

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// BlockStatement
//
class BlockStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(BlockStatement, Statement);
  friend class NodeFactory;

 public:
  const std::vector<Statement*>& statements() const { return statements_; }

 private:
  explicit BlockStatement(Token* keyword,
                          const std::vector<Statement*>& statements);
  ~BlockStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  const std::vector<Statement*> statements_;

  DISALLOW_COPY_AND_ASSIGN(BlockStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_BLOCK_STATEMENT_H_
