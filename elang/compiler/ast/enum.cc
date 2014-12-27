// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/enum.h"

#include "base/logging.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Enum
//
Enum::Enum(NamespaceBody* namespace_body, Modifiers modifiers, Token* keyword,
           Token* name)
    : NamespaceMember(namespace_body, modifiers, keyword, name) {
  DCHECK_EQ(keyword->type(), TokenType::Enum);
  DCHECK(name->is_name());
}

Enum::~Enum() {
}

void Enum::AddMember(EnumMember* member) {
  if (!FindMember(member->name()))
    map_[member->name()->simple_name()] = member;
  members_.push_back(member);
}

EnumMember* Enum::FindMember(Token* name) {
  auto const it = map_.find(name->simple_name());
  return it == map_.end() ? nullptr : it->second;
}


}  // namespace ast
}  // namespace compiler
}  // namespace elang
