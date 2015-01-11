// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace_member.h"

#include "base/logging.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

namespace {
MemberContainer* owner_of(NamespaceBody* namespace_body) {
  return namespace_body ? namespace_body->owner() : nullptr;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
NamespaceMember::NamespaceMember(NamespaceBody* namespace_body,
                                 Modifiers modifiers,
                                 Token* keyword,
                                 Token* name)
    : NamedNode(owner_of(namespace_body), keyword, name),
      WithModifiers(modifiers),
      namespace_body_(namespace_body) {
  DCHECK(namespace_body_ || keyword == TokenType::Namespace);
}

MemberContainer* NamespaceMember::outer() const {
  return parent() ? parent()->as<MemberContainer>() : nullptr;
}

bool NamespaceMember::IsDescendantOf(const NamespaceMember* other) const {
  for (auto runner = outer(); runner; runner = runner->outer()) {
    if (runner == other)
      return true;
  }
  return false;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
