// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/do_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// DoStatement
//
DoStatement::DoStatement(Token* keyword,
                         Statement* statement,
                         Expression* condition)
    : Statement(keyword), condition_(condition), statement_(statement) {
  DCHECK_EQ(keyword, TokenType::Do);
}

DoStatement::~DoStatement() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
