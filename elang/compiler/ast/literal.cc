// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/literal.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Literal
//
Literal::Literal(Token* literal) : Expression(literal) {
}

Literal::~Literal() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
