// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CONTAINER_NODE_H_
#define ELANG_COMPILER_AST_CONTAINER_NODE_H_

#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ContainerNode
//
class ContainerNode : public NamedNode {
  DECLARE_AST_NODE_ABSTRACT_CLASS(ContainerNode, NamedNode);

 public:
  // List of members ordered by source code location.
  const ZoneVector<Node*> members() const { return members_; }
  const ZoneUnorderedMap<AtomicString*, NamedNode*> named_members() const {
    return named_members_;
  }

  // Helper function for visitor pattern. Call |Accept(Visitor*)| for each
  // member.
  void AcceptForMembers(Visitor* visitor);
  virtual void AddMember(Node* member);
  virtual void AddNamedMember(NamedNode* member);
  NamedNode* FindDirectMember(AtomicString* name);
  NamedNode* FindDirectMember(Token* name);
  NamedNode* FindMember(AtomicString* name);
  NamedNode* FindMember(Token* name);

 protected:
  ContainerNode(Zone* zone, ContainerNode* parent, Token* keyword, Token* name);

  virtual NamedNode* FindMemberMore(AtomicString* name);

 private:
  ZoneUnorderedMap<AtomicString*, NamedNode*> named_members_;
  ZoneVector<Node*> members_;

  DISALLOW_COPY_AND_ASSIGN(ContainerNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CONTAINER_NODE_H_
