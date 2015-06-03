// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/class.h"

#include "base/logging.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Class
//
Class::Class(Zone* zone,
             ContainerNode* outer,
             Modifiers modifiers,
             Token* keyword,
             Token* name,
             const std::vector<Type*>& base_class_names)
    : ContainerNode(zone, outer, keyword, name),
      WithModifiers(modifiers),
      base_class_names_(zone, base_class_names) {
}

bool Class::is_class() const {
  return keyword() == TokenType::Class;
}

bool Class::is_interface() const {
  return keyword() == TokenType::Interface;
}

bool Class::is_struct() const {
  return keyword() == TokenType::Struct;
}

#if _DEBUG
// Node
bool Class::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>() || container->is<ast::NamespaceBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Const
//
Const::Const(Class* outer,
             Modifiers modifiers,
             Token* keyword,
             Type* type,
             Token* name,
             Expression* expression)
    : NamedNode(outer, keyword, name),
      WithModifiers(modifiers),
      expression_(expression),
      type_(type) {
  DCHECK_EQ(keyword, TokenType::Const);
  DCHECK_EQ(modifiers, Modifiers::Const() & modifiers);
}

#if _DEBUG
// Node
bool Const::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Field
//
Field::Field(Class* outer,
             Modifiers modifiers,
             Token* keyword,
             Type* type,
             Token* name,
             Expression* expression)
    : NamedNode(outer, keyword, name),
      WithModifiers(modifiers),
      expression_(expression),
      type_(type) {
  DCHECK_EQ(keyword, TokenType::Var);
  DCHECK_EQ(modifiers, Modifiers::Field() & modifiers);
}

#if _DEBUG
// Node
bool Field::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>();
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
