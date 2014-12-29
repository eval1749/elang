// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/named_node.h"

#include "base/logging.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
NamedNode::NamedNode(Token* keyword, Token* name) : Node(keyword), name_(name) {
  DCHECK(name->is_name());
}

NamedNode::~NamedNode() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
