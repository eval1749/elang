// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_DOMINATOR_TREE_BUILDER_H_
#define ELANG_BASE_ANALYSIS_DOMINATOR_TREE_BUILDER_H_

#include <algorithm>
#include <memory>
#include <unordered_map>

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/analysis/dominator_tree.h"
#include "elang/base/graphs/graph.h"
#include "elang/base/graphs/graph_sorter.h"
#include "elang/base/ordered_list.h"
#include "elang/base/zone_owner.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// DominatorTreeEditor
//
template <typename Graph>
class DominatorTreeEditor final : public ZoneOwner {
 public:
  typedef DominatorTree<Graph> DominatorTree;
  typedef typename Graph::Derived GraphNode;

 protected:
  typedef typename DominatorTree::Node TreeNode;

  DominatorTreeEditor();
  ~DominatorTreeEditor() = default;

  std::unique_ptr<DominatorTree> Finish() { return std::move(dominator_tree_); }

  void AddChild(TreeNode* parent, TreeNode* child) {
    DCHECK_NE(parent, child);
    parent->children_.push_back(child);
  }

  void AddFrontier(TreeNode* node, TreeNode* frontier) {
    auto const it =
        std::find(node->frontiers_.begin(), node->frontiers_.end(), frontier);
    if (it != node->frontiers_.end())
      return;
    node->frontiers_.push_back(frontier);
  }

  void InitializeDominatorTree(const OrderedList<GraphNode*>& nodes);
  void SetTreeNodeParent(TreeNode* node, TreeNode* parent, int depth) {
    node->parent_ = parent;
    node->depth_ = depth;
  }
  TreeNode* TreeNodeOf(GraphNode* value) const;

 private:
  std::unique_ptr<DominatorTree> dominator_tree_;

  DISALLOW_COPY_AND_ASSIGN(DominatorTreeEditor);
};

template <typename Graph>
DominatorTreeEditor<Graph>::DominatorTreeEditor()
    : dominator_tree_(new DominatorTree()) {
}

template <typename Graph>
void DominatorTreeEditor<Graph>::InitializeDominatorTree(
    const OrderedList<GraphNode*>& graph_nodes) {
  DCHECK(dominator_tree_);
  for (auto const node : graph_nodes) {
    dominator_tree_->node_map_[node] =
        new (dominator_tree_->zone()) TreeNode(dominator_tree_->zone(), node);
  }
}

template <typename Graph>
typename DominatorTree<Graph>::Node* DominatorTreeEditor<Graph>::TreeNodeOf(
    GraphNode* value) const {
  return dominator_tree_->TreeNodeOf(value);
}

//////////////////////////////////////////////////////////////////////
//
// DominatorTreeBuilder
//
template <typename Graph, typename Direction>
class DominatorTreeBuilder final : public DominatorTreeEditor<Graph> {
 public:
  explicit DominatorTreeBuilder(const Graph* graph);
  ~DominatorTreeBuilder() = default;

  // |zone| for storing |DominatorTree|.
  std::unique_ptr<DominatorTree> Build();

 private:
  int dfs_position_of(TreeNode* node) const;

  void ComputeChildren();
  void ComputeFrontiers();
  void ComputeParentForAll();
  bool ComputeParentForNode(TreeNode* node);
  TreeNode* Intersect(TreeNode* node1, TreeNode* node2);

  TreeNode* entry_node_;
  const Graph* const graph_;
  OrderedList<GraphNode*> graph_nodes_;

  DISALLOW_COPY_AND_ASSIGN(DominatorTreeBuilder);
};

template <typename Graph, typename Direction>
DominatorTreeBuilder<Graph, Direction>::DominatorTreeBuilder(const Graph* graph)
    : entry_node_(nullptr),
      graph_(graph),
      graph_nodes_(
          GraphSorter<Graph, Direction>::SortByReversePostOrder(graph)) {
}

template <typename Graph, typename Direction>
int DominatorTreeBuilder<Graph, Direction>::dfs_position_of(
    TreeNode* node) const {
  return graph_nodes_.position_of(node->value());
}

template <typename Graph, typename Direction>
std::unique_ptr<DominatorTree<Graph>>
DominatorTreeBuilder<Graph, Direction>::Build() {
  InitializeDominatorTree(graph_nodes_);
  entry_node_ = TreeNodeOf(Direction::EntryOf(graph_));
  // Set sentinel
  SetTreeNodeParent(entry_node_, entry_node_, 1);
  ComputeParentForAll();
  SetTreeNodeParent(entry_node_, nullptr, 1);
  ComputeChildren();
  ComputeFrontiers();
  return Finish();
}

template <typename Graph, typename Direction>
void DominatorTreeBuilder<Graph, Direction>::ComputeChildren() {
  for (auto graph_node : graph_nodes_) {
    auto const tree_node = TreeNodeOf(graph_node);
    auto const parent = tree_node->parent();
    if (!parent) {
      DCHECK_EQ(entry_node_, tree_node);
      continue;
    }
    AddChild(parent, tree_node);
  }
}

//  Loop over all basic block which has more than one predecessors.
template <typename Graph, typename Direction>
void DominatorTreeBuilder<Graph, Direction>::ComputeFrontiers() {
  for (auto const graph_node : graph_nodes_) {
    if (!Direction::HasMoreThanOnePredecessors(graph_node))
      continue;
    auto const node = TreeNodeOf(graph_node);
    for (auto const predecessor_graph_node :
         Direction::PredecessorsOf(graph_node)) {
      auto const predecessor = TreeNodeOf(predecessor_graph_node);
      for (auto runner = predecessor; runner != node->parent();
           runner = runner->parent()) {
        AddFrontier(runner, node);
      }
    }
  }
}

template <typename Graph, typename Direction>
void DominatorTreeBuilder<Graph, Direction>::ComputeParentForAll() {
  auto changed = true;
  while (changed) {
    changed = false;
    for (auto const graph_node : graph_nodes_) {
      auto const node = TreeNodeOf(graph_node);
      if (ComputeParentForNode(node))
        changed = true;
    }
  }
}

template <typename Graph, typename Direction>
bool DominatorTreeBuilder<Graph, Direction>::ComputeParentForNode(
    TreeNode* node) {
  for (auto const parent_value : Direction::PredecessorsOf(node->value())) {
    auto candidate = TreeNodeOf(parent_value);
    if (!candidate->parent())
      continue;

    for (auto const predecessor_value :
         Direction::PredecessorsOf(node->value())) {
      auto const predecessor = TreeNodeOf(predecessor_value);
      if (candidate != predecessor && predecessor->parent())
        candidate = Intersect(candidate, predecessor);
    }

    if (node->parent() != candidate) {
      SetTreeNodeParent(node, candidate, candidate->depth() + 1);
      return true;
    }
  }
  return false;
}

template <typename Graph, typename Direction>
typename DominatorTree<Graph>::Node*
DominatorTreeBuilder<Graph, Direction>::Intersect(TreeNode* finger1,
                                                  TreeNode* finger2) {
  while (finger1 != finger2) {
    while (dfs_position_of(finger1) > dfs_position_of(finger2))
      finger1 = finger1->parent();
    while (dfs_position_of(finger2) > dfs_position_of(finger1))
      finger2 = finger2->parent();
  }
  return finger1;
}

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_DOMINATOR_TREE_BUILDER_H_
