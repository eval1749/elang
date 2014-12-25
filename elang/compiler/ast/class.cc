// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/class.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Class
//
Class::Class(Namespace* outer, const Token& keyword, const Token& simple_name)
    : Namespace(outer, keyword, simple_name) {
  DCHECK(keyword.type() == TokenType::Class ||
         keyword.type() == TokenType::Interface ||
         keyword.type() == TokenType::Struct);
}

Class::~Class() {
}

void Class::AddBaseClassName(const QualifiedName& class_name) {
  base_class_names_.push_back(class_name);
}

// NamespaceMember
Namespace* Class::ToNamespace() {
  return nullptr;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
