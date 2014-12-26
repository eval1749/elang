// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(Token* token) : token_(token) {
}

Node::~Node() {
}
}   // namespace ast
}  // namespace compiler
}  // namespace elang
