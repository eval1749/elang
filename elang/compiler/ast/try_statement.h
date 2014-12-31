// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_TRY_STATEMENT_H_
#define ELANG_COMPILER_AST_TRY_STATEMENT_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// TryStatement
//
class TryStatement final : public Statement {
  DECLARE_AST_NODE_CLASS(TryStatement, Statement);

 public:
  BlockStatement* finally_block() const { return finally_block_; }
  const ZoneVector<CatchClause*>& catch_clauses() const {
    return catch_clauses_;
  }
  BlockStatement* protected_block() const { return protected_block_; }

 private:
  TryStatement(Zone* zone,
               Token* keyword,
               BlockStatement* protected_block,
               const std::vector<CatchClause*>& catch_clauses,
               BlockStatement* finally_block);

  // Node
  void Accept(Visitor* visitor) override;

  const ZoneVector<CatchClause*> catch_clauses_;
  BlockStatement* const finally_block_;
  BlockStatement* const protected_block_;

  DISALLOW_COPY_AND_ASSIGN(TryStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_TRY_STATEMENT_H_
