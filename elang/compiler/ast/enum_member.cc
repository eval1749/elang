// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/enum_member.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
EnumMember::EnumMember(Enum* owner, Token* simple_name,
                       Expression* expression)
    : Node(simple_name), expression_(expression) {
  DCHECK(simple_name->is_name());
}

EnumMember::~EnumMember() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
