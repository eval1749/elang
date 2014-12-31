// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_BLOCK_STATEMENT_H_
#define ELANG_COMPILER_AST_BLOCK_STATEMENT_H_

#include <vector>

#include "elang/base/zone_vector.h"
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
  DECLARE_AST_NODE_CLASS(BlockStatement, Statement);

 public:
  const ZoneVector<Statement*>& statements() const { return statements_; }

 private:
  BlockStatement(Zone* zone,
                 Token* keyword,
                 const std::vector<Statement*>& statements);

  // Node
  void Accept(Visitor* visitor) override;

  const ZoneVector<Statement*> statements_;

  DISALLOW_COPY_AND_ASSIGN(BlockStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_BLOCK_STATEMENT_H_
