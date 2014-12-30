// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/alias.h"

#include "base/logging.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Alias
//
Alias::Alias(NamespaceBody* namespace_body,
             Token* keyword,
             Token* name,
             Expression* reference)
    : NamespaceMember(namespace_body, Modifiers(), keyword, name),
      reference_(reference) {
  DCHECK_EQ(keyword->type(), TokenType::Using);
}

Alias::~Alias() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
