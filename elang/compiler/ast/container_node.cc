// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/container_node.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"

namespace elang {
namespace compiler {
namespace ast {

// ContainerNode
ContainerNode::ContainerNode(Zone* zone,
                             ContainerNode* parent,
                             Token* keyword,
                             Token* name)
    : NamedNode(parent, keyword, name), members_(zone) {
}

void ContainerNode::AddMember(Node* member) {
  DCHECK(member->CanBeMemberOf(this)) << *member << " " << *this;
  members_.push_back(member);
}

// NodeTree
Node* ContainerNode::ChildAt(size_t index) const {
  return members_[index];
}

size_t ContainerNode::CountChildNodes() const {
  return members_.size();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
