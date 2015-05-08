// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CONTAINER_NODE_H_
#define ELANG_COMPILER_AST_CONTAINER_NODE_H_

#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/nodes.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ContainerNode
//
class ContainerNode : public NamedNode {
  DECLARE_ABSTRACT_AST_NODE_CLASS(ContainerNode, NamedNode);

 public:
  // List of members ordered by source code location.
  const ZoneVector<Node*> members() const { return members_; }
  const ZoneUnorderedMap<AtomicString*, NamedNode*> named_members() const {
    return named_members_;
  }

  // Helper function for visitor pattern. Call |Accept(Visitor*)| for each
  // member.
  void AcceptForMembers(Visitor* visitor);
  void AddMember(Node* member);
  void AddNamedMember(NamedNode* member);
  NamedNode* FindMember(AtomicString* name) const;
  NamedNode* FindMember(Token* name) const;

 protected:
  ContainerNode(Zone* zone, ContainerNode* parent, Token* keyword, Token* name);

 private:
  ZoneVector<Node*> members_;
  ZoneUnorderedMap<AtomicString*, NamedNode*> named_members_;

  DISALLOW_COPY_AND_ASSIGN(ContainerNode);
};

//////////////////////////////////////////////////////////////////////
//
// BodyNode
//
class BodyNode : public ContainerNode {
  DECLARE_ABSTRACT_AST_NODE_CLASS(BodyNode, ContainerNode);

 public:
  NamespaceNode* owner() const { return owner_; }

 protected:
  BodyNode(Zone* zone, BodyNode* parent, NamespaceNode* owner);

 private:
  NamespaceNode* const owner_;

  DISALLOW_COPY_AND_ASSIGN(BodyNode);
};

//////////////////////////////////////////////////////////////////////
//
// NamespaceNode
//
class NamespaceNode : public ContainerNode {
  DECLARE_ABSTRACT_AST_NODE_CLASS(NamespaceNode, ContainerNode);

 protected:
  NamespaceNode(Zone* zone, ContainerNode* outer, Token* keyword, Token* name);

 private:
  DISALLOW_COPY_AND_ASSIGN(NamespaceNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CONTAINER_NODE_H_
