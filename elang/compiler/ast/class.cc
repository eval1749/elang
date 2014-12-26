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
    : Namespace(outer, keyword, simple_name), is_fixed_(false) {
  DCHECK(keyword.type() == TokenType::Class ||
         keyword.type() == TokenType::Interface ||
         keyword.type() == TokenType::Struct);
}

Class::~Class() {
}

const std::vector<Class*>& Class::base_classes() const {
  DCHECK(is_fixed_);
  return base_classes_;
}

void Class::AddBaseClassName(const QualifiedName& class_name) {
  DCHECK(!is_fixed_);
  base_class_names_.push_back(class_name);
}

void Class::BindBaseClasses(const std::vector<Class*>& base_classes) {
  DCHECK(!is_fixed_);
  DCHECK(base_classes_.empty());
  DCHECK_EQ(base_classes.size(), base_class_names_.size());
#if _DEBUG
  // We check |base_classes[0]| is proper class rather than |struct|,
  // |interface|.
  auto first = true;
  for (auto const base_class : base_classes) {
    if (first) {
      DCHECK(base_class->token().type() == TokenType::Class ||
             base_class->token().type() == TokenType::Interface);
      first = false;
      continue;
    }
    DCHECK_EQ(base_class->token().type(), TokenType::Interface);
  }
#endif
  base_classes_ = base_classes;
  is_fixed_ = true;
}

// NamespaceMember
Namespace* Class::ToNamespace() {
  return nullptr;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
