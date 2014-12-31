// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/binary_operation.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// BinaryOperation
//
BinaryOperation::BinaryOperation(Token* op, Expression* left, Expression* right)
    : Expression(op), left_(left), right_(right) {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
