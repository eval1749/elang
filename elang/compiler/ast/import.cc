// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/import.h"

#include "base/logging.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Import
//
Import::Import(NamespaceBody* namespace_body,
               Token* keyword,
               Expression* reference)
    : NamespaceMember(namespace_body, keyword, reference->token()),
      reference_(reference) {
  DCHECK_EQ(keyword, TokenType::Using);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
