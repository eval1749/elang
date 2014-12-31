// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/block_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// BlockStatement
//
BlockStatement::BlockStatement(Zone* zone,
                               Token* keyword,
                               const std::vector<Statement*>& statements)
    : Statement(keyword), statements_(zone, statements) {
  DCHECK_EQ(keyword, TokenType::LeftCurryBracket);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
