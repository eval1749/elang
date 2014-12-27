// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/method.h"

#include "base/logging.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Method
//
Method::Method(NamespaceBody* namespace_body, Modifiers modifiers,
               Expression* return_type, Token* name,
               const std::vector<Token*>& type_parameters,
               const Parameters& parameters)
    : NamespaceMember(namespace_body, modifiers, name, name),
      parameters_(parameters), return_type_(return_type),
      type_parameters_(type_parameters) {
  DCHECK(name->is_name());
}

Method::~Method() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
