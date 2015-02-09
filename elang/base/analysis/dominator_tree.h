// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_DOMINATOR_TREE_H_
#define ELANG_BASE_ANALYSIS_DOMINATOR_TREE_H_

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/base_export.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"

namespace elang {

template <typename Graph>
class DominatorTreeEditor;

//////////////////////////////////////////////////////////////////////
//
// DominatorTree represent dominator tree on |Graph|.
// To constructor dominator tree, please use |DominatorTreeBuilder|.
//
template <typename Graph>
class DominatorTree final : public ZoneOwner {
 public:
  typedef typename Graph::Derived GraphNode;
  class Node;
  typedef ZoneVector<Node*> Nodes;

  class Node : public ZoneAllocated {
   public:
    ~Node() = delete;

    const Nodes& children() const { return children_; }
    int depth() const { return depth_; }
    const Nodes& frontiers() const { return frontiers_; }
    Node* parent() const { return parent_; }
    GraphNode* value() const { return value_; }

   private:
    // DominatorTreeEditor uses |Node| constructor.
    friend class DominatorTreeEditor<Graph>;

    Node(Zone* zone, GraphNode* value);

    Nodes children_;
    int depth_;
    Nodes frontiers_;
    Node* parent_;
    GraphNode* const value_;

    DISALLOW_COPY_AND_ASSIGN(Node);
  };

  ~DominatorTree() = default;

  // Returns true if |dominator| dominates |dominatee|.
  bool Dominates(GraphNode* dominator, GraphNode* dominatee) const;

  // Returns |Node| associated to |graph_node|
  Node* TreeNodeOf(GraphNode* graph_node) const;

 private:
  friend class DominatorTreeEditor<Graph>;

  DominatorTree();

  ZoneUnorderedMap<GraphNode*, Node*> node_map_;

  DISALLOW_COPY_AND_ASSIGN(DominatorTree);
};

//////////////////////////////////////////////////////////////////////
//
// DominatorTree::Node
//
template <typename Graph>
DominatorTree<Graph>::Node::Node(Zone* zone, GraphNode* graph_node)
    : children_(zone),
      depth_(0),
      frontiers_(zone),
      parent_(nullptr),
      value_(graph_node) {
}

//////////////////////////////////////////////////////////////////////
//
// DominatorTree
//
template <typename Graph>
DominatorTree<Graph>::DominatorTree()
    : node_map_(zone()) {
}

template <typename Graph>
bool DominatorTree<Graph>::Dominates(GraphNode* dominator,
                                     GraphNode* dominatee) const {
  auto const dominator_node = TreeNodeOf(dominator);
  auto const dominatee_node = TreeNodeOf(dominatee);
  for (auto runner = dominatee_node; runner; runner = runner->parent()) {
    if (runner == dominator_node)
      return true;
  }
  return false;
}

template <typename Graph>
typename DominatorTree<Graph>::Node* DominatorTree<Graph>::TreeNodeOf(
    GraphNode* graph_node) const {
  auto const it = node_map_.find(graph_node);
  DCHECK(it != node_map_.end());
  return it->second;
}

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_DOMINATOR_TREE_H_
