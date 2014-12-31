// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/if_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// IfStatement
//
IfStatement::IfStatement(Token* keyword,
                         Expression* condition,
                         Statement* then_statement,
                         Statement* else_statement)
    : Statement(keyword),
      condition_(condition),
      else_statement_(else_statement),
      then_statement_(then_statement) {
  DCHECK_EQ(keyword, TokenType::If);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
