// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/constructed_type.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ConstructedType
//
ConstructedType::ConstructedType(Expression* type,
                                 const std::vector<Expression*>& args)
    : Expression(type->token()), arguments_(args), blueprint_type_(type) {
  DCHECK(!arguments_.empty());
}

ConstructedType::~ConstructedType() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
