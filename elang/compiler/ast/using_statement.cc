// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/using_statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"
#include "elang/compiler/ast/local_variable.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// UsingStatement
//
UsingStatement::UsingStatement(Token* keyword,
                               LocalVariable* variable,
                               Expression* resource,
                               Statement* statement)
    : Statement(keyword),
      resource_(resource),
      statement_(statement),
      variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Using);
  DCHECK(!variable_ || variable_->value() == resource_);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
