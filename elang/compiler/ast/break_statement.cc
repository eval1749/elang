// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/break_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// BreakStatement
//
BreakStatement::BreakStatement(Token* keyword) : Statement(keyword) {
  DCHECK_EQ(keyword, TokenType::Break);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
