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
VarStatement::VarStatement(Zone* zone,
                           Token* type_token,
                           const std::vector<LocalVariable*>& variables)
    : Statement(type_token), variables_(zone, variables) {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
