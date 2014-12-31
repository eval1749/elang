// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/unary_operation.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// UnaryOperation
//
UnaryOperation::UnaryOperation(Token* op, Expression* expression)
    : Expression(op), expression_(expression) {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
