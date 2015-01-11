// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node.h"

#include "base/logging.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(Token* token) : token_(token) {
}

Token* Node::name() const {
  return token();
}

void Node::Accept(Visitor* visitor) {
  __assume(visitor);
  NOTREACHED();
}

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
NamedNode::NamedNode(Token* keyword, Token* name) : Node(keyword), name_(name) {
  DCHECK(name->is_name());
}

Token* NamedNode::name() const {
  return name_;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
