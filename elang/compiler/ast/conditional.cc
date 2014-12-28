// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/conditional.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Conditional
//
Conditional::Conditional(Token* op,
                         Expression* cond_expr,
                         Expression* then_expr,
                         Expression* else_expr)
    : Expression(op), cond_(cond_expr), else_(else_expr), then_(then_expr) {
}

Conditional::~Conditional() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
