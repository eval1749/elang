// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/expression_statement.h"

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ExpressionStatement
//
ExpressionStatement::ExpressionStatement(Expression* expression)
    : Statement(expression->token()), expression_(expression) {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
