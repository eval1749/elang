// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_LOOP_TREE_H_
#define ELANG_BASE_ANALYSIS_LOOP_TREE_H_

#include <stdint.h>
#include <memory>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"

namespace elang {

template <typename Graph>
class LoopTreeEditor;

//////////////////////////////////////////////////////////////////////
//
// LoopTree
//
template <typename Graph>
class LoopTree final : public ZoneOwner {
 public:
  using GraphNode = typename Graph::Derived;

  // See "loop_tree_builder.h" for implementation of |Editor|.
  class Editor;
  class Iterator;
  class TreeNode;

  class Iterator final {
   public:
    Iterator(const LoopTree* tree, TreeNode* node);
    Iterator(const Iterator& other);
    ~Iterator() = default;

    Iterator& operator=(const Iterator& other);
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const { return !operator==(other); }

    TreeNode* operator->() const { return operator*(); }
    TreeNode* operator*() const;
    Iterator& operator++();

   private:
    TreeNode* node_;
    const LoopTree* const tree_;
  };

  class TreeNode final : public ZoneAllocated {
   public:
    enum class Kind {
      MultipleEntryLoop,
      Root,
      SingleEntryLoop,
    };

    ~TreeNode() = delete;

    const ZoneVector<TreeNode*>& children() const { return children_; }
    int depth() const { return depth_; }
    GraphNode* entry() const { return const_cast<GraphNode*>(entry_); }
    bool is_multiple_entry() const { return kind_ == Kind::MultipleEntryLoop; }
    bool is_root() const { return kind_ == Kind::Root; }
    bool is_single_entry() const { return kind_ == Kind::SingleEntryLoop; }
    Kind kind() const { return kind_; }
    const ZoneVector<GraphNode*>& nodes() const { return nodes_; }
    TreeNode* parent() const { return parent_; }

   private:
    friend class Editor;

    TreeNode(Zone* zone, Kind kind, const GraphNode* entry);

    ZoneVector<TreeNode*> children_;
    int depth_;
    const GraphNode* const entry_;
    Kind const kind_;
    ZoneVector<GraphNode*> nodes_;
    TreeNode* parent_;

    DISALLOW_COPY_AND_ASSIGN(TreeNode);
  };

  LoopTree();
  ~LoopTree() = default;

  Iterator begin() const { return Iterator(this, root_); }
  Iterator end() const { return Iterator(this, nullptr); }

  TreeNode* NodeOf(const GraphNode* node) const;

 private:
  ZoneUnorderedMap<const GraphNode*, TreeNode*> map_;
  TreeNode* root_;

  DISALLOW_COPY_AND_ASSIGN(LoopTree);
};

// LoopTree::Iterator
template <typename Graph>
LoopTree<Graph>::Iterator::Iterator(const LoopTree* tree, TreeNode* node)
    : node_(node), tree_(tree) {
}

template <typename Graph>
LoopTree<Graph>::Iterator::Iterator(const Iterator& other)
    : Iterator(other.tree_, other.node_) {
}

template <typename Graph>
typename LoopTree<Graph>::Iterator& LoopTree<Graph>::Iterator::operator=(
    const Iterator& other) {
  DCHECK_EQ(tree_, other.tree_);
  node_ = other.node_;
  return *this;
}

template <typename Graph>
bool LoopTree<Graph>::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(tree_, other.tree_);
  return node_ == other.node_;
}

template <typename Graph>
typename LoopTree<Graph>::TreeNode* LoopTree<Graph>::Iterator::operator*()
    const {
  DCHECK(node_);
  return node_;
}

template <typename Graph>
typename LoopTree<Graph>::Iterator& LoopTree<Graph>::Iterator::operator++() {
  DCHECK(node_);
  if (node_->children().size()) {
    node_ = node_->children().front();
    return *this;
  }
  for (;;) {
    auto const parent = node_->parent();
    if (!parent) {
      node_ = nullptr;
      return *this;
    }
    auto& children = parent->children();
    auto it = std::find(children.begin(), children.end(), node_);
    DCHECK(it != children.end());
    ++it;
    if (it != children.end()) {
      node_ = *it;
      return *this;
    }
    node_ = parent;
  }
}

// LoopTree::TreeNode
template <typename Graph>
LoopTree<Graph>::TreeNode::TreeNode(Zone* zone,
                                    Kind kind,
                                    const GraphNode* node)
    : children_(zone),
      depth_(0),
      entry_(node),
      kind_(kind),
      nodes_(zone),
      parent_(nullptr) {
}

// LoopTree
template <typename Graph>
LoopTree<Graph>::LoopTree()
    : map_(zone()), root_(nullptr) {
}

template <typename Graph>
typename LoopTree<Graph>::TreeNode* LoopTree<Graph>::NodeOf(
    const GraphNode* node) const {
  auto const it = map_.find(node);
  DCHECK(it != map_.end());
  return it->second;
}

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_LOOP_TREE_H_
