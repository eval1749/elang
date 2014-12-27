// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/assignment.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Assignment
//
Assignment::Assignment(Token* op, Expression* left, Expression* right)
    : Expression(op), left_(left), right_(right) {
}

Assignment::~Assignment() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
