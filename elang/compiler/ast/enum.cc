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
           BodyNode* outer,
           Modifiers modifiers,
           Token* keyword,
           Token* name,
           Type* enum_base)
    : NamespaceNode(zone, outer, keyword, name),
      WithModifiers(modifiers),
      enum_base_(enum_base) {
  DCHECK_EQ(keyword->type(), TokenType::Enum);
  DCHECK(name->is_name());
  DCHECK_EQ(modifiers, Modifiers::Enum() & modifiers);
}

#if _DEBUG
// Node
bool Enum::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::ClassBody>() || container->is<ast::NamespaceBody>();
}

// NamedNode
bool Enum::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>() || container->is<ast::Namespace>() ||
         CanBeMemberOf(container);
}
#endif

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
EnumMember::EnumMember(Enum* owner,
                       Token* name,
                       int position,
                       Expression* expression)
    : NamedNode(owner, name, name),
      expression_(expression),
      position_(position) {
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
