// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/local_variable.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Variable
//
Variable::Variable(Token* keyword,
                   Expression* type,
                   Token* name,
                   Expression* value)
    : NamedNode(nullptr, keyword, name), type_(type), value_(value) {
  DCHECK(!keyword || keyword == TokenType::Const ||
         keyword == TokenType::Catch || keyword == TokenType::For ||
         keyword == TokenType::Using);
}

bool Variable::is_const() const {
  return token() == TokenType::Const || token() == TokenType::Using;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
