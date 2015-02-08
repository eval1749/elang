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
    Derived* next() const { return next_; }
    Derived* previous() const { return previous_; }

   protected:
#if _DEBUG
    Node() : next_(nullptr), owner_(nullptr), previous_(nullptr) {}
#else
    Node() : next_(nullptr), previous_(nullptr) {}
#endif
    ~Node() = default;

   private:
    friend class DoubleLinked;

    Derived* next_;
    Derived* previous_;
#if _DEBUG
    DoubleLinked* owner_;
#endif

    DISALLOW_COPY_AND_ASSIGN(Node);
  };

  class Iterator final : public IteratorBase {
   public:
    explicit Iterator(Derived* node) : IteratorBase(node) {}
    Iterator(const Iterator& other) = default;
    ~Iterator() = default;

    Iterator& operator=(const Iterator& other) = default;

    Iterator& operator++() {
      DCHECK(!!current_);
      current_ = static_cast<Node*>(current_)->next_;
      return *this;
    }
  };

  class ReverseIterator final : public IteratorBase {
   public:
    explicit ReverseIterator(Derived* node) : IteratorBase(node) {}
    ReverseIterator(const ReverseIterator& other) = default;
    ~ReverseIterator() = default;

    ReverseIterator& operator=(const ReverseIterator& other) = default;

    ReverseIterator& operator++() {
      DCHECK(!!current_);
      current_ = static_cast<Node*>(current_)->previous_;
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
  Derived* first_node() const { return first_; }
  Derived* last_node() const { return last_; }
  ReverseIterator rbegin() { return ReverseIterator(last_); }
  ReverseIterator rend() { return ReverseIterator(nullptr); }
  Reversed reversed() { return Reversed(this); }

  // Inserts |new_derived| at the end of this list. |new_derived| must not be
  // in this list.
  void AppendNode(Derived* new_derived) {
    auto const new_node = static_cast<Node*>(new_derived);
#if _DEBUG
    DCHECK(!new_node->owner_) << "new node should not be in this list.";
    new_node->owner_ = this;
#endif
    DCHECK(!new_node->next_);
    DCHECK(!new_node->previous_);
    DCHECK_NE(first_, new_node);

    new_node->next_ = nullptr;
    new_node->previous_ = last_;
    if (!first_)
      first_ = new_derived;
    if (last_)
      static_cast<Node*>(last_)->next_ = new_derived;
    last_ = new_derived;
  }

  // Returns number of elements in this list. This takes O(n), where n is
  // number of node in this list.
  int Count() const {
    auto num_nodes = 0;
    for (auto it = begin(); it != end(); ++it)
      ++num_nodes;
    return num_nodes;
  }

  // Inserts |new_derived| after |ref_derived|. |new_derived| must not be in
  // this list.
  void InsertAfter(Derived* new_derived, Derived* ref_derived) {
    DCHECK_NE(new_derived, ref_derived);

    if (!ref_derived) {
      PrependNode(new_derived);
      return;
    }
    auto const new_node = static_cast<Node*>(new_derived);
#if _DEBUG
    DCHECK(!new_node->owner_) << "new node should not be in this list.";
    new_node->owner_ = this;
#endif
    DCHECK(!new_node->next_);
    DCHECK(!new_node->previous_);
    DCHECK_NE(first_, new_node);

    auto const ref_node = static_cast<Node*>(ref_derived);
#if _DEBUG
    DCHECK_EQ(this, ref_node->owner_) << "ref node must be in this list.";
    new_node->owner_ = this;
#endif

    auto const next = ref_node->next_;
    if (next)
      static_cast<Node*>(next)->previous_ = new_derived;
    else
      last_ = new_derived;

    new_node->next_ = next;
    new_node->previous_ = ref_derived;
    ref_node->next_ = new_derived;
  }

  // Inserts |new_derived| before |ref_derived|. |new_derived| must not be in
  // this list.
  void InsertBefore(Derived* new_derived, Derived* ref_derived) {
    DCHECK_NE(new_derived, ref_derived);

    if (!ref_derived) {
      AppendNode(new_derived);
      return;
    }
    auto const new_node = static_cast<Node*>(new_derived);
#if _DEBUG
    DCHECK(!new_node->owner_) << "new node should not be in this list.";
    new_node->owner_ = this;
#endif
    DCHECK(!new_node->next_);
    DCHECK(!new_node->previous_);
    DCHECK_NE(first_, new_node);

    auto const ref_node = static_cast<Node*>(ref_derived);
#if _DEBUG
    DCHECK_EQ(this, ref_node->owner_) << "ref node must be in this list.";
#endif

    auto const previous = ref_node->previous_;
    if (previous)
      static_cast<Node*>(previous)->next_ = new_derived;
    else
      first_ = new_derived;

    new_node->next_ = ref_derived;
    new_node->previous_ = previous;
    ref_node->previous_ = new_derived;
  }

  // Inserts |new_derived| at first.
  void PrependNode(Derived* new_derived) {
    auto const new_node = static_cast<Node*>(new_derived);
#if _DEBUG
    DCHECK(!new_node->owner_) << "new node should not be in this list.";
    new_node->owner_ = this;
#endif
    DCHECK(!new_node->next_) << "new node should not be in this list.";
    DCHECK(!new_node->previous_);
    DCHECK_NE(first_, new_node);

    new_node->next_ = first_;
    new_node->previous_ = nullptr;
    if (!last_)
      last_ = new_derived;
    if (first_)
      static_cast<Node*>(first_)->previous_ = new_derived;
    first_ = new_derived;
  }

  // Remove all nodes in this list.
  void RemoveAll() {
    while (first_)
      RemoveNode(first_);
  }

  // Removes |old_derived| from this list.
  void RemoveNode(Derived* old_derived) {
    auto const old_node = static_cast<Node*>(old_derived);
#if _DEBUG
    DCHECK_EQ(this, old_node->owner_) << "old_node must be in this list.";
    old_node->owner_ = nullptr;
#endif
    auto const next = old_node->next_;
    auto const previous = old_node->previous_;

    if (next)
      static_cast<Node*>(next)->previous_ = previous;
    else
      last_ = previous;

    if (previous)
      static_cast<Node*>(previous)->next_ = next;
    else
      first_ = next;

    old_node->next_ = nullptr;
    old_node->previous_ = nullptr;
  }

  // Replaces |old_derived| by |new_derived|. |new_derived| should not be in
  // this list.
  void ReplaceNode(Derived* new_derived, Derived* old_derived) {
    DCHECK_NE(new_derived, old_derived);

    auto const new_node = static_cast<Node*>(new_derived);
#if _DEBUG
    DCHECK(!new_node->owner_) << "new node should not be in this list.";
    new_node->owner_ = this;
#endif
    DCHECK(!new_node->next_) << "new node should not be in this list.";
    DCHECK(!new_node->previous_);
    DCHECK_NE(first_, new_node);

    auto const old_node = static_cast<Node*>(old_derived);
#if _DEBUG
    DCHECK_EQ(this, old_node->owner_) << "old node must be in this list.";
    old_node->owner_ = nullptr;
#endif

    auto const next = old_node->next_;
    auto const previous = old_node->previous_;
    old_node->next_ = nullptr;
    old_node->previous_ = nullptr;

    if (next)
      static_cast<Node*>(next)->previous_ = new_derived;
    else
      last_ = new_derived;
    new_node->next_ = next;

    if (previous)
      static_cast<Node*>(previous)->next_ = new_derived;
    else
      first_ = new_derived;

    new_node->previous_ = previous;
  }

 private:
  class IteratorBase {
   public:
    Derived* operator*() const { return current_; }
    Derived* operator->() const { return current_; }
    bool operator==(const IteratorBase& other) const {
      return current_ == other.current_;
    }
    bool operator!=(const IteratorBase& other) const {
      return !operator==(other);
    }

   protected:
    explicit IteratorBase(Derived* node) : current_(node) {}
    ~IteratorBase() = default;

    Derived* current_;
  };

  Derived* first_;
  Derived* last_;
  DISALLOW_COPY_AND_ASSIGN(DoubleLinked);
};

}  // namespace elang

#endif  // ELANG_BASE_DOUBLE_LINKED_H_
