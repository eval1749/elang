// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_DOUBLE_LINKED_H_
#define ELANG_BASE_DOUBLE_LINKED_H_

#include "base/macros.h"
#include "base/logging.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// DoubleLinked
// provides O(1) insertion and deletion of node in list. A node of list must
// be derived from |DoubleLinked<T, U>::Node| with |public| accessibility.
//
// Example:
//  class MyItem : public DoubleLinked<MyItem, MyItemCollection> {
//    ...
//  };
//
//  class MyItemCollection {
//    ...
//   private:
//    DoubleLinked<MyItem, MyItemCollection> items_;
//  };
//
//  Following functions are available:
//    begin()
//    empty()
//    end()
//    first_node()
//    last_node()
//    rbegin()
//    rend()
//    reversed() for last to first iteration in ranged-for loop.
//    AppendNode(Node* new_node)
//    InsertAfter(Node* new_node, Node* ref_node)
//    InsertBefore(Node* new_node, Node* ref_node)
//    PrependNode(Node* new_node)
//    RemoveAll()
//    RemoveNode(Node* old_node)
//    ReplaceNode(Derive* new_node, Derive* old_node);
//
//  Note:
//    |old_node| must be in a list. |ref_node| must be in a list or null.
//    Unlike DOM API |new_node| must be not in a list.
//
template <typename Derived, typename AnchorType>
class DoubleLinked final {
 public:
  class Node {
   public:
    Derived* next() const { return static_cast<Derived*>(next_); }
    Derived* previous() const { return static_cast<Derived*>(previous_); }

   protected:
#if _DEBUG
    Node() : next_(nullptr), owner_(nullptr), previous_(nullptr) {}
#else
    Node() : next_(nullptr), previous_(nullptr) {}
#endif
    ~Node() = default;

   private:
    friend class DoubleLinked;

    Node* next_;
    Node* previous_;
#if _DEBUG
    DoubleLinked* owner_;
#endif

    DISALLOW_COPY_AND_ASSIGN(Node);
  };

  class Iterator final : public IteratorBase {
   public:
    explicit Iterator(Node* node) : IteratorBase(node) {}
    Iterator(const Iterator& other) = default;
    ~Iterator() = default;

    Iterator& operator=(const Iterator& other) = default;

    Iterator& operator++() {
      DCHECK(!!current_);
      current_ = current_->next_;
      return *this;
    }
  };

  class ReverseIterator final : public IteratorBase {
   public:
    explicit ReverseIterator(Node* node) : IteratorBase(node) {}
    ReverseIterator(const ReverseIterator& other) = default;
    ~ReverseIterator() = default;

    ReverseIterator& operator=(const ReverseIterator& other) = default;

    ReverseIterator& operator++() {
      DCHECK(!!current_);
      current_ = current_->previous_;
      return *this;
    }
  };

  // Used for last to first loop in range-for loop, e.g.
  //    for (auto const node : list.reversed()) {
  //      result += node->value();
  //    }
  class Reversed final {
   public:
    explicit Reversed(DoubleLinked<Derived, AnchorType>* anchor)
        : anchor_(anchor) {}
    ~Reversed() = default;

    ReverseIterator begin() const { return anchor_->rbegin(); }
    ReverseIterator end() const { return anchor_->rend(); }

   private:
    DoubleLinked* anchor_;
  };

  DoubleLinked() : first_(nullptr), last_(nullptr) {}
  ~DoubleLinked() = default;

  Iterator begin() const { return Iterator(first_); }
  bool empty() const { return !first_; }
  Iterator end() const { return Iterator(nullptr); }
  Derived* first_node() const { return static_cast<Derived*>(first_); }
  Derived* last_node() const { return static_cast<Derived*>(last_); }
  ReverseIterator rbegin() { return ReverseIterator(last_); }
  ReverseIterator rend() { return ReverseIterator(nullptr); }
  Reversed reversed() { return Reversed(this); }

  void AppendNode(Derived* derived) {
    auto const node = static_cast<Node*>(derived);
    DCHECK(!node->next_);
    DCHECK(!node->previous_);
    DCHECK(!node->owner_);
#if _DEBUG
    node->owner_ = this;
#endif
    node->next_ = nullptr;
    node->previous_ = last_;
    if (!first_)
      first_ = node;
    if (last_)
      last_->next_ = node;
    last_ = node;
  }

  int Count() const {
    auto num_nodes = 0;
    for (auto it = begin(); it != end(); ++it)
      ++num_nodes;
    return num_nodes;
  }

  void InsertAfter(Derived* new_derived, Derived* ref_derived) {
    if (!ref_derived) {
      PrependNode(new_derived);
      return;
    }
    auto const new_node = static_cast<Node*>(new_derived);
    DCHECK(!new_node->next_);
    DCHECK(!new_node->previous_);
    auto const ref_node = static_cast<Node*>(ref_derived);
#if _DEBUG
    DCHECK(!new_node->owner_);
    DCHECK_EQ(this, ref_node->owner_);
    new_node->owner_ = this;
#endif
    auto const next = ref_node->next_;
    if (next)
      next->previous_ = new_node;
    else
      last_ = new_node;
    new_node->next_ = next;
    new_node->previous_ = ref_node;
    ref_node->next_ = new_node;
  }

  void InsertBefore(Derived* new_derived, Derived* ref_derived) {
    if (!ref_derived) {
      AppendNode(new_derived);
      return;
    }
    auto const new_node = static_cast<Node*>(new_derived);
    DCHECK(!new_node->previous_);
    DCHECK(!new_node->next_);
    auto const ref_node = static_cast<Node*>(ref_derived);
#if _DEBUG
    DCHECK(!new_node->owner_);
    DCHECK_EQ(this, ref_node->owner_);
    new_node->owner_ = this;
#endif
    auto const previous = ref_node->previous_;
    if (previous)
      previous->next_ = new_node;
    else
      first_ = new_node;
    new_node->next_ = ref_node;
    new_node->previous_ = previous;
    ref_node->previous_ = new_node;
  }

  void RemoveAll() {
    while (first_)
      RemoveNode(static_cast<Derived*>(first_));
  }

  void RemoveNode(Derived* old_derived) {
    auto const node = static_cast<Node*>(old_derived);
#if _DEBUG
    DCHECK_EQ(this, node->owner_);
    node->owner_ = nullptr;
#endif
    auto const next = node->next_;
    auto const previous = node->previous_;
    if (next)
      next->previous_ = previous;
    else
      last_ = previous;
    if (previous)
      previous->next_ = next;
    else
      first_ = next;
    node->next_ = nullptr;
    node->previous_ = nullptr;
  }

  void PrependNode(Derived* new_derived) {
    auto const node = static_cast<Node*>(new_derived);
    DCHECK(!node->next_);
    DCHECK(!node->previous_);
#if _DEBUG
    DCHECK(!node->owner_);
    node->owner_ = this;
#endif
    node->next_ = first_;
    node->previous_ = nullptr;
    if (!last_)
      last_ = node;
    if (first_)
      first_->previous_ = node;
    first_ = node;
  }

 private:
  class IteratorBase {
   public:
    Derived* operator*() const { return static_cast<Derived*>(current_); }
    Derived* operator->() const { return static_cast<Derived*>(current_); }
    bool operator==(const IteratorBase& other) const {
      return current_ == other.current_;
    }
    bool operator!=(const IteratorBase& other) const {
      return !operator==(other);
    }

   protected:
    explicit IteratorBase(Node* node) : current_(node) {}
    ~IteratorBase() = default;

    Node* current_;
  };

  Node* first_;
  Node* last_;
  DISALLOW_COPY_AND_ASSIGN(DoubleLinked);
};

}  // namespace elang

#endif  // ELANG_BASE_DOUBLE_LINKED_H_
