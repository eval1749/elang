// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "elang/compiler/ast/nodes.h"

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

// ChildNodes
template <typename T>
ChildNodes<T>::ChildNodes(const Node* node, size_t start)
    : node_(node), start_(start) {
}

template <typename T>
ChildNodes<T>::ChildNodes(const ChildNodes& other)
    : ChildNodes(other.node_, other.start_) {
}

template <typename T>
ChildNodes<T>::~ChildNodes() {
}

template <typename T>
ChildNodes<T>& ChildNodes<T>::operator=(const ChildNodes& other) {
  node_ = other.node_;
  return *this;
}

template <typename T>
typename ChildNodes<T>::Iterator ChildNodes<T>::begin() const {
  return Iterator(node_, start_);
}

template <typename T>
typename ChildNodes<T>::Iterator ChildNodes<T>::end() const {
  return Iterator(node_, node_->CountChildNodes());
}

template <typename T>
size_t ChildNodes<T>::size() const {
  auto const end = node_->CountChildNodes();
  DCHECK_LE(start_, end);
  return end - start_;
}

// ChildNodes<T>::Iterator
template <typename T>
ChildNodes<T>::Iterator::Iterator(const Node* node, size_t index)
    : index_(index), node_(node) {
}

template <typename T>
ChildNodes<T>::Iterator::Iterator(const Iterator& other)
    : Iterator(other.node_, other.index_) {
}

template <typename T>
ChildNodes<T>::Iterator::~Iterator() {
}

template <typename T>
typename ChildNodes<T>::Iterator& ChildNodes<T>::Iterator::operator=(
    const Iterator& other) {
  node_ = other.node_;
  index_ = other.index_;
  return *this;
}

template <typename T>
bool ChildNodes<T>::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(node_, other.node_);
  return index_ == other.index_;
}

template <typename T>
bool ChildNodes<T>::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

template <typename T>
T* ChildNodes<T>::Iterator::operator*() const {
  return node_->ChildAt(index_)->as<T>();
}

template <typename T>
T* ChildNodes<T>::Iterator::operator->() const {
  return operator*();
}

template <typename T>
typename ChildNodes<T>::Iterator& ChildNodes<T>::Iterator::operator++() {
  DCHECK_LT(index_, node_->CountChildNodes()) << node_;
  ++index_;
  return *this;
}

template class ChildNodes<CatchClause>;
template class ChildNodes<Expression>;
template class ChildNodes<Node>;
template class ChildNodes<Statement>;
template class ChildNodes<VarDeclaration>;

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(Node* parent, Token* token) : parent_(parent), token_(token) {
  // Since we use |token| for sorting error message, we should have non-null
  // token.
  DCHECK(token);
}

ChildNodes<Node> Node::child_nodes() const {
  return ChildNodes<Node>(this, 0);
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

// NodeTree
Node* Node::ChildAt(size_t index) const {
  NOTREACHED() << this << " " << index;
  return nullptr;
}

size_t Node::CountChildNodes() const {
  return 0;
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
  DCHECK(name->is_name()) << name;
}

Token* NamedNode::name() const {
  return name_;
}

base::string16 NamedNode::NewQualifiedName() const {
  std::vector<AtomicString*> ancestors;
  size_t length = 0u;
  for (const ast::Node* runner = this; runner; runner = runner->parent()) {
    if (!runner->parent())
      break;
    auto const name_token = runner->name();
    DCHECK(name_token->has_atomic_string());
    if (!ancestors.empty())
      ++length;
    ancestors.push_back(name_token->atomic_string());
    length += ancestors.back()->string().length();
  }
  std::reverse(ancestors.begin(), ancestors.end());
  base::string16 buffer(length, '?');
  buffer.resize(0);
  auto first = true;
  for (auto const runner : ancestors) {
    if (first)
      first = false;
    else
      buffer.push_back('.');
    runner->string().AppendToString(&buffer);
  }
  return buffer;
}

// NodeTree
NodeTree::NodeTree() {
}

NodeTree::~NodeTree() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
