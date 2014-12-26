// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/expression.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Expression
//
Expression::Expression(Token* operator_token,
                       const std::vector<Expression*>& operands)
    : Node(operator_token), operands_(operands) {
}

Expression::Expression(Token* operator_token, Expression* operand0,
                       Expression* operand1)
    : Expression(operator_token,
                 std::vector<Expression*> { operand0, operand1 }) {
}

Expression::Expression(Token* operator_token, Expression* operand0)
    : Expression(operator_token, std::vector<Expression*> { operand0 }) {
}

Expression::Expression(Token* operator_token)
    : Expression(operator_token, std::vector<Expression*>()) {
}

Expression::~Expression() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
