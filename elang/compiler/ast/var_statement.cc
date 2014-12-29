// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/var_statement.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// VarStatement
//
VarStatement::VarStatement(Token* keyword,
                           const std::vector<LocalVariable*>& variables)
    : Statement(keyword), variables_(variables) {
}

VarStatement::~VarStatement() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
