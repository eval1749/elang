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
    : NamedNode(parent, keyword, name), members_(zone), named_members_(zone) {
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
  auto const name = member->name()->atomic_string();
  // We keep first member declaration.
  if (named_members_.count(name))
    return;
  named_members_[name] = member;
}

NamedNode* ContainerNode::FindMember(AtomicString* name) const {
  auto const it = named_members_.find(name);
  return it == named_members_.end() ? nullptr : it->second;
}

NamedNode* ContainerNode::FindMember(Token* name) const {
  return FindMember(name->atomic_string());
}

// BodyNode
BodyNode::BodyNode(Zone* zone, BodyNode* parent, NamespaceNode* owner)
    : ContainerNode(zone, parent, owner->keyword(), owner->name()),
      owner_(owner) {
  DCHECK(owner->is<ast::Class>() || owner->is<ast::Namespace>());
}

// NamespaceNode
NamespaceNode::NamespaceNode(Zone* zone,
                             ContainerNode* outer,
                             Token* keyword,
                             Token* name)
    : ContainerNode(zone, outer, keyword, name) {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
