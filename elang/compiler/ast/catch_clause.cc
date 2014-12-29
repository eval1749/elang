// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/catch_clause.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// CatchClause
//
CatchClause::CatchClause(Token* keyword,
                         Expression* type,
                         LocalVariable* variable,
                         BlockStatement* block)
    : Node(keyword), block_(block), type_(type), variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Catch);
  DCHECK(block_);
}

CatchClause::~CatchClause() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
