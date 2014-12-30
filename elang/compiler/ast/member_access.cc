// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/member_access.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// MemberAccess
//
MemberAccess::MemberAccess(const std::vector<Expression*>& members)
    : Expression(members.front()->token()), members_(members) {
  DCHECK_GE(members.size(), 2u);
}

MemberAccess::~MemberAccess() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
