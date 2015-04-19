// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_LOOP_TREE_BUILDER_OLD_H_
#define ELANG_BASE_ANALYSIS_LOOP_TREE_BUILDER_OLD_H_

#include <vector>
#include <unordered_map>
#include <utility>

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/disjoint_sets.h"
#include "elang/base/analysis/loop_tree.h"
#include "elang/base/graphs/graph_sorter.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_vector.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// LoopTree<Graph>::Editor
//
template <typename Graph>
class LoopTree<Graph>::Editor {
 public:
  Editor() = default;
  ~Editor();

  void AddChildNode(TreeNode* parent, TreeNode* child);
  void AddGraphNode(TreeNode* tree_node, GraphNode* component);
  LoopTree Finalize(const GraphNode* entry_node);
  TreeNode* NewTreeNode(typename TreeNode::Kind kind, GraphNode* entry);

 private:
  void AssignTreeNode(const GraphNode* node, TreeNode* tree_node);
  bool IsFinalized() const { return !loop_tree_.IsAlive(); }

  LoopTree loop_tree_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

template <typename Graph>
LoopTree<Graph>::Editor::~Editor() {
  DCHECK(IsFinalized());
  DCHECK(loop_tree_.map_.empty());
}

template <typename Graph>
void LoopTree<Graph>::Editor::AddChildNode(TreeNode* parent, TreeNode* child) {
  DCHECK(!IsFinalized());
  DCHECK(!child->parent_);
  DCHECK(std::find(parent->children_.begin(), parent->children_.end(), child) ==
         parent->children_.end());
  parent->children_.push_back(child);
  child->parent_ = parent;
}

template <typename Graph>
void LoopTree<Graph>::Editor::AddGraphNode(TreeNode* tree_node,
                                           GraphNode* component) {
  DCHECK(!IsFinalized());
  DCHECK(std::find(tree_node->nodes_.begin(), tree_node->nodes_.end(),
                   component) == tree_node->nodes_.end());
  tree_node->nodes_.push_back(component);
  AssignTreeNode(component, tree_node);
}

template <typename Graph>
void LoopTree<Graph>::Editor::AssignTreeNode(const GraphNode* node,
                                             TreeNode* tree_node) {
  DCHECK(!IsFinalized());
  DCHECK(!loop_tree_.map_.count(node));
  loop_tree_.map_.insert(std::make_pair(node, tree_node));
}

template <typename Graph>
LoopTree<Graph> LoopTree<Graph>::Editor::Finalize(const GraphNode* entry_node) {
  DCHECK(!IsFinalized());
  auto const root = loop_tree_.NodeOf(entry_node);
  DCHECK(root->is_root());

  // Set depth of loop tree nodes.
  std::vector<TreeNode*> queue(root->children().begin(),
                               root->children().end());
  while (!queue.empty()) {
    auto const tree_node = queue.back();
    queue.pop_back();
    tree_node->depth_ = tree_node->parent_->depth_ + 1;
    queue.insert(queue.end(), tree_node->children().begin(),
                 tree_node->children().end());
  }

  return std::move(loop_tree_);
}

template <typename Graph>
typename LoopTree<Graph>::TreeNode* LoopTree<Graph>::Editor::NewTreeNode(
    typename TreeNode::Kind kind,
    GraphNode* entry) {
  DCHECK(!IsFinalized());
  auto const zone = loop_tree_.zone();
  auto const tree_node = new (zone) TreeNode(zone, kind, entry);
  AssignTreeNode(entry, tree_node);
  return std::move(tree_node);
}

//////////////////////////////////////////////////////////////////////
//
// LoopTreeBuilder
//
template <typename Graph>
class LoopTreeBuilder final : public ZoneOwner {
 public:
  using DominatorTree = DominatorTree<Graph>;
  using LoopTree = LoopTree<Graph>;

  LoopTreeBuilder(const Graph* graph, const DominatorTree& dominator_tree);
  ~LoopTreeBuilder() = default;

  LoopTree Build();

 private:
  using GraphNode = typename Graph::Derived;
  using GraphNodeSet = std::unordered_set<const GraphNode*>;
  using LoopTreeNode = typename LoopTree::TreeNode;
  using LoopNodeKind = typename LoopTreeNode::Kind;

  class LoopNode : public ZoneAllocated {
   public:
    LoopNode(Zone* zone, const GraphNode* entry);
    ~LoopNode() = delete;

    const ZoneVector<LoopNode*> components() const { return components_; }
    const GraphNode* entry() const { return entry_; }
    LoopTreeNode* tree_node() const { return tree_node_; }
    void set_tree_node(LoopTreeNode* tree_node);

    void AddComponent(LoopNode* node) { components_.push_back(node); }

   private:
    ZoneVector<LoopNode*> components_;
    const GraphNode* const entry_;
    LoopNode* parent_;
    LoopTreeNode* tree_node_;

    DISALLOW_COPY_AND_ASSIGN(LoopNode);
  };

  void FindBody(LoopNodeKind kind,
                const GraphNode* node,
                const GraphNodeSet& generators);
  void FindLoop(const GraphNode* node);
  bool IsBackEdge(const GraphNode* from, const GraphNode* to) const;
  LoopNode* LoopNodeOf(const GraphNode* node);

  const DominatorTree& dominator_tree_;
  typename LoopTree::Editor editor_;
  std::unordered_map<const GraphNode*, GraphNodeSet> generators_;
  std::unordered_map<const GraphNode*, LoopNode*> map_;
  OrderedList<GraphNode*> post_order_list_;
  DisjointSets<LoopNode*> sets_;

  DISALLOW_COPY_AND_ASSIGN(LoopTreeBuilder);
};

// LoopTreeBuilder::LoopNode
template <typename Graph>
LoopTreeBuilder<Graph>::LoopNode::LoopNode(Zone* zone, const GraphNode* entry)
    : components_(zone), entry_(entry), parent_(nullptr), tree_node_(nullptr) {
}

template <typename Graph>
void LoopTreeBuilder<Graph>::LoopNode::set_tree_node(LoopTreeNode* tree_node) {
  DCHECK(tree_node);
  DCHECK(!tree_node_);
  tree_node_ = tree_node;
}

// LoopTreeBuilder
template <typename Graph>
LoopTreeBuilder<Graph>::LoopTreeBuilder(const Graph* graph,
                                        const DominatorTree& dominator_tree)
    : dominator_tree_(dominator_tree),
      post_order_list_(GraphSorter<Graph>::SortByPostOrder(graph)) {
}

template <typename Graph>
LoopTree<Graph> LoopTreeBuilder<Graph>::Build() {
  for (auto node : post_order_list_) {
    auto const loop_node = new (zone()) LoopNode(zone(), node);
    sets_.MakeSet(loop_node);
    map_.insert(std::make_pair(node, loop_node));
  }

  for (auto node : post_order_list_) {
    auto const generators = generators_[node];
    if (!generators.empty())
      FindBody(LoopNodeKind::MultipleEntryLoop, node, generators);
    FindLoop(node);
  }
  auto const entry_node = post_order_list_.front();
  auto const exit_node = post_order_list_.back();
  FindBody(LoopNodeKind::Root, entry_node, {exit_node});
  return std::move(editor_.Finalize(entry_node));
}

template <typename Graph>
void LoopTreeBuilder<Graph>::FindBody(LoopNodeKind kind,
                                      const GraphNode* node,
                                      const GraphNodeSet& generators) {
  auto const loop_node = LoopNodeOf(node);
  std::vector<LoopNode*> queue;
  for (auto const generator : generators) {
    auto const component = LoopNodeOf(generator);
    if (sets_.InSameSet(loop_node, component))
      continue;
    sets_.Union(loop_node, component);
    queue.push_back(component);
    loop_node->AddComponent(component);
  }
  while (!queue.empty()) {
    auto const node = queue.back();
    queue.pop_back();
    for (auto predecessor : node->entry()->predecessors()) {
      if (predecessor == loop_node->entry())
        continue;
      auto const component = LoopNodeOf(predecessor);
      if (sets_.InSameSet(loop_node, component))
        continue;
      sets_.Union(loop_node, component);
      queue.push_back(component);
      loop_node->AddComponent(component);
    }
  }

  auto const tree_node =
      editor_.NewTreeNode(kind, const_cast<GraphNode*>(loop_node->entry()));
  loop_node->set_tree_node(tree_node);
  for (auto component : loop_node->components()) {
    auto const child_tree_node = component->tree_node();
    if (child_tree_node) {
      editor_.AddChildNode(tree_node, child_tree_node);
      continue;
    }
    DCHECK(component->components().empty());
    editor_.AddGraphNode(tree_node, const_cast<GraphNode*>(component->entry()));
  }
}

template <typename Graph>
void LoopTreeBuilder<Graph>::FindLoop(const GraphNode* to) {
  GraphNodeSet components;
  auto loop_entry = to;
  for (auto from : to->predecessors()) {
    if (!IsBackEdge(from, to))
      continue;
    loop_entry = dominator_tree_.CommonAncestorOf(from, loop_entry);
    components.insert(from);
  }

  if (loop_entry != to) {
    generators_[loop_entry].insert(components.begin(), components.end());
    return;
  }
  FindBody(LoopNodeKind::SingleEntryLoop, loop_entry, components);
}

template <typename Graph>
bool LoopTreeBuilder<Graph>::IsBackEdge(const GraphNode* from,
                                        const GraphNode* to) const {
  if (from == to)
    return true;
  auto const from_node = dominator_tree_.TreeNodeOf(from);
  auto const to_node = dominator_tree_.TreeNodeOf(to);
  return from_node->position() < to_node->position();
}

template <typename Graph>
typename LoopTreeBuilder<Graph>::LoopNode* LoopTreeBuilder<Graph>::LoopNodeOf(
    const GraphNode* node) {
  auto const it = map_.find(node);
  DCHECK(it != map_.end());
  return it->second;
}

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_LOOP_TREE_BUILDER_OLD_H_
