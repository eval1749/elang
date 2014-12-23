// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/enum.h"

#include "base/logging.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Enum
//
Enum::Enum(Namespace* outer, const Token& keyword,
           const Token& simple_name)
    : NamespaceMember(outer, keyword, simple_name) {
  DCHECK_EQ(keyword.type(), TokenType::Enum);
  DCHECK(simple_name.is_name());
}

Enum::~Enum() {
}


void Enum::AddMember(EnumMember* member) {
  if (!FindMember(member->simple_name()))
    map_[member->simple_name().id()] = member;
  members_.push_back(member);
}

EnumMember* Enum::FindMember(const Token& simple_name) {
  auto const it = map_.find(simple_name.id());
  return it == map_.end() ? nullptr : it->second;
}


}  // namespace ast
}  // namespace compiler
}  // namespace elang
