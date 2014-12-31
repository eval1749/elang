// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(Token* token) : token_(token) {
}

void Node::Accept(Visitor* visitor) {
  __assume(visitor);
  NOTREACHED();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
