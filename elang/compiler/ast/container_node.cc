// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/container_node.h"

#include "base/logging.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ContainerNode
//
ContainerNode::ContainerNode(Zone* zone,
                             ContainerNode* parent,
                             Token* keyword,
                             Token* name)
    : NamedNode(parent, keyword, name), named_members_(zone), members_(zone) {
}

void ContainerNode::AcceptForMembers(Visitor* visitor) {
  for (auto const member : members_)
    member->Accept(visitor);
}

void ContainerNode::AddMember(Node* member) {
  DCHECK(member->CanBeMemberOf(this));
  members_.push_back(member);
}

void ContainerNode::AddNamedMember(NamedNode* member) {
  DCHECK(member->CanBeNamedMemberOf(this));
  // We keep first member declaration.
  if (FindMember(member->name()))
    return;
  named_members_[member->name()->simple_name()] = member;
}

NamedNode* ContainerNode::FindMember(AtomicString* name) const {
  auto const it = named_members_.find(name);
  return it == named_members_.end() ? nullptr : it->second;
}

NamedNode* ContainerNode::FindMember(Token* name) const {
  return FindMember(name->simple_name());
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
