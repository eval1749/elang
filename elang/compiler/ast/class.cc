// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/class.h"

#include "base/logging.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Class
//
Class::Class(Zone* zone,
             NamespaceBody* namespace_body,
             Modifiers modifiers,
             Token* keyword,
             Token* name)
    : MemberContainer(zone, namespace_body, modifiers, keyword, name),
      base_class_names_(zone) {
  DCHECK(keyword == TokenType::Class || keyword == TokenType::Interface ||
         keyword == TokenType::Struct);
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

// Node
bool Class::is_type() const {
  return true;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
