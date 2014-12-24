// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/alias.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Alias
//
Alias::Alias(Namespace* outer, const Token& keyword, const Token& simple_name,
             const QualifiedName& target_name)
    : NamespaceMember(outer, keyword, simple_name), target_name_(target_name) {
  DCHECK_EQ(keyword.type(), TokenType::Using);
}

Alias::~Alias() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
