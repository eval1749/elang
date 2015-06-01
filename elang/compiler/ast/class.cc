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
             NamespaceNode* outer,
             Modifiers modifiers,
             Token* keyword,
             Token* name)
    : NamespaceNode(zone, outer, keyword, name), WithModifiers(modifiers) {
  DCHECK(keyword == TokenType::Class || keyword == TokenType::Interface ||
         keyword == TokenType::Struct);
  DCHECK_EQ(modifiers, Modifiers::Class() & modifiers);
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

//////////////////////////////////////////////////////////////////////
//
// ClassBody
//
ClassBody::ClassBody(Zone* zone,
                     BodyNode* outer,
                     Modifiers modifiers,
                     Token* keyword,
                     Token* name,
                     const std::vector<Type*>& base_class_names)
    : BodyNode(zone, outer, keyword, name),
      WithModifiers(modifiers),
      base_class_names_(zone, base_class_names) {
}

bool ClassBody::is_class() const {
  return keyword() == TokenType::Class;
}

bool ClassBody::is_interface() const {
  return keyword() == TokenType::Interface;
}

bool ClassBody::is_struct() const {
  return keyword() == TokenType::Struct;
}

#if _DEBUG
// Node
bool ClassBody::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::ClassBody>() || container->is<ast::NamespaceBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Const
//
Const::Const(ClassBody* outer,
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
  return container->is<ast::ClassBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Field
//
Field::Field(ClassBody* outer,
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
  return container->is<ast::ClassBody>();
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
