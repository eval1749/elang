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
           Token* name,
           Type* enum_base)
    : ContainerNode(zone, outer, keyword, name),
      WithModifiers(modifiers),
      enum_base_(enum_base) {
  DCHECK_EQ(keyword->type(), TokenType::Enum);
  DCHECK(name->is_name());
  DCHECK_EQ(modifiers, Modifiers::Enum() & modifiers);
}

#if _DEBUG
// Node
bool Enum::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>() || container->is<ast::NamespaceBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
EnumMember::EnumMember(Enum* owner,
                       Token* name,
                       Expression* explicit_expression,
                       Expression* implicit_expression)
    : NamedNode(owner, name, name),
      explicit_expression_(explicit_expression),
      implicit_expression_(implicit_expression) {
  DCHECK(name->is_name());
  if (explicit_expression_)
    DCHECK(!implicit_expression_);
  else
    DCHECK(implicit_expression_);
}

Enum* EnumMember::owner() const {
  return parent()->as<Enum>();
}

#if _DEBUG
// Node
bool EnumMember::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Enum>();
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
