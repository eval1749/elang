// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node.h"

#include "base/logging.h"
#include "elang/compiler/ast/container_node.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(ContainerNode* parent, Token* token)
    : parent_(parent), token_(token) {
}

bool Node::is_type() const {
  return false;
}

Token* Node::name() const {
  return token();
}

bool Node::CanBeInNamespaceBody() const {
  return false;
}

bool Node::IsDescendantOf(const Node* other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == other)
      return true;
  }
  return false;
}

// Visitable<Visitor>
void Node::Accept(Visitor* visitor) {
  __assume(visitor);
  NOTREACHED();
}

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
NamedNode::NamedNode(ContainerNode* parent, Token* keyword, Token* name)
    : Node(parent, keyword), name_(name) {
  DCHECK(name->is_name());
}

Token* NamedNode::name() const {
  return name_;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
