// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/array_type.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ArrayType
//
ArrayType::ArrayType(Token* op, Expression* element_type,
                     const std::vector<int>& ranks)
    : Expression(op), element_type_(element_type), ranks_(ranks) {
}

ArrayType::~ArrayType() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
