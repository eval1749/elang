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

  void AddMember(Node* member);

 protected:
  ContainerNode(Zone* zone, ContainerNode* parent, Token* keyword, Token* name);

 private:
  Node* ChildAt(size_t index) const final;
  size_t CountChildNodes() const final;

  ZoneVector<Node*> members_;

  DISALLOW_COPY_AND_ASSIGN(ContainerNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CONTAINER_NODE_H_
