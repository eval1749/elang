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
// LocalVariable
//
LocalVariable::LocalVariable(Token* keyword,
                             Expression* type,
                             Token* name,
                             Expression* value)
    : Node(name), keyword_(keyword), type_(type), value_(value) {
  DCHECK(!keyword || keyword == TokenType::Const ||
         keyword == TokenType::Catch);
}

LocalVariable::~LocalVariable() {
}

bool LocalVariable::is_const() const {
  return keyword_ == TokenType::Const;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang