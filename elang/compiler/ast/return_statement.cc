// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/return_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ReturnStatement
//
ReturnStatement::ReturnStatement(Token* keyword, Expression* value)
    : Statement(keyword), value_(value) {
  DCHECK_EQ(keyword, TokenType::Return);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
