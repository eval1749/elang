// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NODES_H_
#define ELANG_COMPILER_AST_NODES_H_

#include <memory>

#include "elang/compiler/ast/nodes_forward.h"

namespace elang {
namespace compiler {
namespace ast {

#define DECLARE_AST_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);      \
  friend class Factory;                     \
                                            \
 protected:                                 \
  ~self() = default;

#define DECLARE_ABSTRACT_AST_NODE_CLASS(self, super) \
  DECLARE_AST_NODE_CLASS(self, super);

#define DECLARE_CONCRETE_AST_NODE_CLASS(self, super) \
  DECLARE_AST_NODE_CLASS(self, super);               \
  void Accept(Visitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable, public Visitable<Visitor>, public ZoneAllocated {
  DECLARE_ABSTRACT_AST_NODE_CLASS(Node, Castable);

 public:
  // Returns true if C++ class represents type, e.g. |ArrayType|, |Class|,
  // |Enum|, and so on. For |MemberAccess| and |NameReference| always return
  // false.
  virtual bool is_type() const;

  // Associated name like thing for error message and debug log.
  virtual Token* name() const;

  ContainerNode* parent() const { return parent_; }

  // A token which parser created this not for.
  Token* token() const { return token_; }

#if _DEBUG
  virtual bool CanBeMemberOf(ContainerNode* container) const;
#endif

  bool IsDescendantOf(const Node* other) const;

  // Visitable<Visitor>
  // Default implementation for node classes not in
  // |FOR_EACH_CONCRETE_AST_NODE()|.
  void Accept(Visitor* visitor) override;

 protected:
  Node(ContainerNode* parent, Token* token);

 private:
  ContainerNode* const parent_;
  Token* const token_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
class NamedNode : public Node {
  DECLARE_ABSTRACT_AST_NODE_CLASS(NamedNode, Node);

 public:
  Token* keyword() const { return token(); }
  Token* name() const override;

#if _DEBUG
  virtual bool CanBeNamedMemberOf(ContainerNode* container) const;
#endif

 protected:
  NamedNode(ContainerNode* parent, Token* keyword, Token* name);

 private:
  Token* const name_;

  DISALLOW_COPY_AND_ASSIGN(NamedNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODES_H_
