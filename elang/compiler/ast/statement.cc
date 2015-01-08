// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/statement.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Statement
//
Statement::Statement(Token* op) : Node(op) {
}

ExpressionList::ExpressionList(Token* keyword,
                               const std::vector<Expression*>& expressions)
    : Statement(keyword), expressions_(expressions) {
}

ForEachStatement::ForEachStatement(Token* keyword,
                                   LocalVariable* variable,
                                   Expression* enumerable,
                                   Statement* statement)
    : Statement(keyword),
      enumerable_(enumerable),
      statement_(statement),
      variable_(variable) {
}

ForStatement::ForStatement(Token* keyword,
                           Statement* initializer,
                           Expression* condition,
                           Statement* step,
                           Statement* statement)
    : Statement(keyword),
      condition_(condition),
      initializer_(initializer),
      statement_(statement),
      step_(step) {
}

InvalidStatement::InvalidStatement(Token* token) : Statement(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
