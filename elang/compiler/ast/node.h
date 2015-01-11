// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NODE_H_
#define ELANG_COMPILER_AST_NODE_H_

#include <memory>

#include "elang/compiler/ast/nodes_forward.h"

namespace elang {
namespace compiler {
namespace ast {

#define DECLARE_AST_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);      \
  friend class NodeFactory;                 \
                                            \
 protected:                                 \
  ~self() = default;

#define DECLARE_AST_NODE_CONCRETE_CLASS(self, super) \
  DECLARE_AST_NODE_CLASS(self, super);               \
  void Accept(Visitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable, public Visitable<Visitor>, public ZoneAllocated {
  DECLARE_AST_NODE_CLASS(Node, Castable);

 public:
  // Returns true if C++ class represents type, e.g. |ArrayType|, |Class|,
  // |Enum|, and so on. For |MemberAccess| and |NameReference| always return
  // false.
  virtual bool is_type() const;

  // Associated name like thing for error message and debug log.
  virtual Token* name() const;

  // A token which parser created this not for.
  Token* token() const { return token_; }

  // Visitable<Visitor>
  // Default implementation for node classes not in |FOR_EACH_AST_NODE()|.
  void Accept(Visitor* visitor) override;

 protected:
  explicit Node(Token* token);

 private:
  Token* const token_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
class NamedNode : public Node {
  DECLARE_AST_NODE_CLASS(NamedNode, Node);

 public:
  Token* keyword() const { return token(); }
  Token* name() const override;
  Node* parent() const { return parent_; }

 protected:
  // TODO(eval1749) |parent| should be |ContainerNode|.
  NamedNode(Node* parent, Token* keyword, Token* name);

 private:
  Token* const name_;
  Node* const parent_;

  DISALLOW_COPY_AND_ASSIGN(NamedNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODE_H_
