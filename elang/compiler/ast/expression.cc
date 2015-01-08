// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/expression.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Expression
//
Expression::Expression(Token* op) : Node(op) {
}

InvalidExpression::InvalidExpression(Token* token) : Expression(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
