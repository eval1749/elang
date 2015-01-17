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
    : NamespaceNode(zone, outer, keyword, name),
      WithModifiers(modifiers),
      base_class_names_(zone) {
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

void Class::AddBaseClassName(Expression* class_name) {
  base_class_names_.push_back(class_name);
}

#if _DEBUG
// NamedNode
bool Class::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>() || container->is<ast::Namespace>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// ClassBody
//
ClassBody::ClassBody(Zone* zone, BodyNode* outer, Class* owner)
    : BodyNode(zone, outer, owner) {
}

Class* ClassBody::owner() const {
  return BodyNode::owner()->as<ast::Class>();
}

#if _DEBUG
// Node
bool ClassBody::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>() || container->is<ast::ClassBody>() ||
         container->is<ast::NamespaceBody>();
}

// NamedNode
bool ClassBody::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::BodyNode>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Field
//
Field::Field(ClassBody* outer,
             Modifiers modifiers,
             Type* type,
             Token* name,
             Expression* expression)
    : NamedNode(outer, name, name),
      WithModifiers(modifiers),
      expression_(expression),
      type_(type) {
  DCHECK_EQ(modifiers, Modifiers::Field() & modifiers);
}

#if _DEBUG
// Node
bool Field::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::ClassBody>();
}

// NamedNode
bool Field::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>() || container->is<ast::ClassBody>();
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
