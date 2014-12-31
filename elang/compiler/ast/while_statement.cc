// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/while_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// WhileStatement
//
WhileStatement::WhileStatement(Token* keyword,
                               Expression* condition,
                               Statement* statement)
    : Statement(keyword), condition_(condition), statement_(statement) {
  DCHECK_EQ(keyword, TokenType::While);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
