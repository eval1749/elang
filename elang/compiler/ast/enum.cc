// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/enum.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Enum
//
Enum::Enum(Zone* zone,
           ContainerNode* outer,
           Modifiers modifiers,
           Token* keyword,
           Token* name)
    : ContainerNode(zone, outer, keyword, name), WithModifiers(modifiers) {
  DCHECK_EQ(keyword->type(), TokenType::Enum);
  DCHECK(name->is_name());
  DCHECK_EQ(modifiers, Modifiers::Enum() & modifiers);
}

// Node
bool Enum::is_type() const {
  return true;
}

#if _DEBUG
// Node
bool Enum::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::NamespaceBody>() || container->is<ast::Class>();
}

// NamedNode
bool Enum::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::Namespace>() || CanBeMemberOf(container);
}
#endif

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
EnumMember::EnumMember(Enum* owner, Token* name, Expression* expression)
    : NamedNode(owner, name, name), expression_(expression) {
  DCHECK(name->is_name());
}

#if _DEBUG
// Node
bool EnumMember::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Enum>();
}

// NamedNode
bool EnumMember::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::Enum>();
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
