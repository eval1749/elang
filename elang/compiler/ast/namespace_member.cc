// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace_member.h"

#include "base/logging.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
NamespaceMember::NamespaceMember(Namespace* outer,
                                 const Token& keyword_or_name,
                                 const Token& simple_name)
    : Node(keyword_or_name),
      alias_declaration_space_(outer ? outer->namespace_body() :
                                       nullptr),
      outer_(outer), simple_name_(simple_name) {
  DCHECK(simple_name.is_name());
  if (outer) {
    DCHECK(alias_declaration_space_);
  } else {
    DCHECK(keyword_or_name.type() == TokenType::Namespace);
  }
}

NamespaceMember::~NamespaceMember() {
}

Namespace* NamespaceMember::ToNamespace() {
  return nullptr;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
