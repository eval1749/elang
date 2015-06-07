// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NODES_H_
#define ELANG_COMPILER_AST_NODES_H_

#include <array>
#include <iterator>
#include <memory>
#include <vector>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/nodes_forward.h"

namespace elang {
// TODO(eval1749) Once our tool chain C++14 |std::is_final<T>|, we don't need
// to define |elang::is_final<T>| for |Node|s.
#define V(Name, ...) \
  template <>        \
  struct is_final<compiler::ast::Name> : std::true_type {};
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V
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
// NodeTree
//
class NodeTree {
 public:
  virtual Node* ChildAt(size_t index) const = 0;
  virtual size_t CountChildNodes() const = 0;

 protected:
  NodeTree();
  virtual ~NodeTree();

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeTree);
};

//////////////////////////////////////////////////////////////////////
//
// Nodes
//
template <typename T>
class ChildNodes final {
 public:
  class Iterator final : public std::iterator<std::forward_iterator_tag, Node> {
   public:
    Iterator(const Iterator& other);
    ~Iterator();

    Iterator& operator=(const Iterator& other);

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

    T* operator*() const;
    T* operator->() const;

    Iterator& operator++();

   private:
    friend class ChildNodes;

    explicit Iterator(const Node* node, size_t index);

    size_t index_;
    const Node* node_;
  };

  explicit ChildNodes(const Node* node);
  ChildNodes(const ChildNodes& other);
  ~ChildNodes();

  ChildNodes& operator=(const ChildNodes& other);

  Iterator begin() const;
  Iterator end() const;

 private:
  const Node* node_;
};

extern template class ChildNodes<Node>;
extern template class ChildNodes<Expression>;
extern template class ChildNodes<Statement>;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable<Node>,
             public NodeTree,
             public Visitable<Visitor>,
             public ZoneAllocated {
  DECLARE_ABSTRACT_AST_NODE_CLASS(Node, Castable);

 public:
  ChildNodes<Node> child_nodes() const;

  // Associated name like thing for error message and debug log.
  virtual Token* name() const;

  Node* parent() const { return parent_; }

  // A token which parser created this not for.
  Token* token() const { return token_; }

#if _DEBUG
  virtual bool CanBeMemberOf(ContainerNode* container) const;
#endif

  bool IsDescendantOf(const Node* other) const;

  // NodeTree
  Node* ChildAt(size_t index) const override;
  size_t CountChildNodes() const override;

  // Visitable<Visitor>
  // Default implementation for node classes not in
  // |FOR_EACH_CONCRETE_AST_NODE()|.
  void Accept(Visitor* visitor) override;

 protected:
  Node(Node* parent, Token* token);

 private:
  Node* parent_;
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

  base::string16 NewQualifiedName() const;

 protected:
  NamedNode(ContainerNode* parent, Token* keyword, Token* name);

 private:
  Token* const name_;

  DISALLOW_COPY_AND_ASSIGN(NamedNode);
};

// SimpleNode
template <typename Base, size_t NumberOfChildNodes>
class SimpleNode : public Base {
 protected:
  template <typename... Params>
  explicit SimpleNode(Params... params)
      : Base(params...) {}
  ~SimpleNode() override {}

  Node* child_at(size_t index) const { return child_nodes_[index]; }

  void set_child_at(size_t index, Node* node) {
    DCHECK(node) << this << " " << index;
    child_nodes_[index] = node;
  }

  // NodeTree
  Node* ChildAt(size_t index) const final {
    DCHECK_LT(index, NumberOfChildNodes) << this;
    return child_at(index);
  }

  size_t CountChildNodes() const final { return NumberOfChildNodes; }

 private:
  std::array<Node*, NumberOfChildNodes> child_nodes_;

  DISALLOW_COPY_AND_ASSIGN(SimpleNode);
};

// VariadicNode
template <typename Base>
class VariadicNode : public Base {
 protected:
  template <typename T, typename... Params>
  explicit VariadicNode(Zone* zone,
                        const std::vector<T>& nodes,
                        Params... params)
      : Base(params...), child_nodes_(zone) {
    child_nodes_.assign(nodes.begin(), nodes.end());
  }
  ~VariadicNode() override {}

  Node* child_at(size_t index) const { return child_nodes_[index]; }

  // NodeTree
  Node* ChildAt(size_t index) const final { return child_at(index); }
  size_t CountChildNodes() const final { return child_nodes_.size(); }

 private:
  ZoneVector<Node*> child_nodes_;

  DISALLOW_COPY_AND_ASSIGN(VariadicNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODES_H_
