// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/try_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// TryStatement
//
TryStatement::TryStatement(Token* keyword,
                           BlockStatement* protected_block,
                           const std::vector<CatchClause*>& catch_clauses,
                           BlockStatement* finally_block)
    : Statement(keyword),
      catch_clauses_(catch_clauses),
      finally_block_(finally_block),
      protected_block_(protected_block) {
  DCHECK_EQ(keyword, TokenType::Try);
}

TryStatement::~TryStatement() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
