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
             Token* simple_name,
             const QualifiedName& target_name)
    : NamespaceMember(namespace_body, Modifiers(), keyword, simple_name),
      target_(nullptr),
      target_name_(target_name) {
  DCHECK_EQ(keyword->type(), TokenType::Using);
}

Alias::~Alias() {
}

void Alias::BindTo(NamespaceMember* target) {
  DCHECK(target);
  DCHECK(!target_);
  target_ = target;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
