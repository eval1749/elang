// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/nodes.h"

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

#if _DEBUG
bool Node::CanBeMemberOf(ContainerNode* container) const {
  DCHECK(container);
  return false;
}
#endif

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

#if _DEBUG
bool NamedNode::CanBeNamedMemberOf(ContainerNode* container) const {
  DCHECK(container);
  return false;
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
