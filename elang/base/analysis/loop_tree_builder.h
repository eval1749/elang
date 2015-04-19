// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_LOOP_TREE_BUILDER_H_
#define ELANG_BASE_ANALYSIS_LOOP_TREE_BUILDER_H_

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/disjoint_sets.h"
#include "elang/base/analysis/loop_tree.h"
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
  Editor();
  ~Editor();

  void AddChildNode(TreeNode* parent, TreeNode* child);
  void AddGraphNode(TreeNode* tree_node, const GraphNode* component);
  std::unique_ptr<LoopTree> Finalize(const GraphNode* entry_node);
  TreeNode* NewTreeNode(typename TreeNode::Kind kind, const GraphNode* entry);

 private:
  void AssignTreeNode(const GraphNode* node, TreeNode* tree_node);
  bool IsFinalized() const { return !loop_tree_; }

  std::unique_ptr<LoopTree> loop_tree_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

template <typename Graph>
LoopTree<Graph>::Editor::Editor()
    : loop_tree_(new LoopTree()) {
}

template <typename Graph>
LoopTree<Graph>::Editor::~Editor() {
  DCHECK(IsFinalized());
}

template <typename Graph>
void LoopTree<Graph>::Editor::AddChildNode(TreeNode* parent, TreeNode* child) {
  DCHECK(!IsFinalized());
  DCHECK(!child->parent_);
  DCHECK_NE(parent, child);
  DCHECK(child->children().empty());
  DCHECK(std::find(parent->children_.begin(), parent->children_.end(), child) ==
         parent->children_.end());
  parent->children_.push_back(child);
  child->parent_ = parent;
  child->depth_ = parent->depth() + 1;
}

template <typename Graph>
void LoopTree<Graph>::Editor::AddGraphNode(TreeNode* tree_node,
                                           const GraphNode* component) {
  DCHECK(!IsFinalized());
  DCHECK_NE(tree_node->entry(), component);
  DCHECK(std::find(tree_node->nodes_.begin(), tree_node->nodes_.end(),
                   component) == tree_node->nodes_.end());
  tree_node->nodes_.push_back(const_cast<GraphNode*>(component));
  AssignTreeNode(component, tree_node);
}

template <typename Graph>
void LoopTree<Graph>::Editor::AssignTreeNode(const GraphNode* node,
                                             TreeNode* tree_node) {
  DCHECK(!IsFinalized());
  DCHECK(!loop_tree_->map_.count(node));
  loop_tree_->map_.insert(std::make_pair(node, tree_node));
}

template <typename Graph>
std::unique_ptr<LoopTree<Graph>> LoopTree<Graph>::Editor::Finalize(
    const GraphNode* entry_node) {
  DCHECK(!IsFinalized());
  auto const root = loop_tree_->NodeOf(entry_node);
  DCHECK(root->is_root());
  loop_tree_->root_ = root;

#if 0
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
#endif

  return std::move(loop_tree_);
}

template <typename Graph>
typename LoopTree<Graph>::TreeNode* LoopTree<Graph>::Editor::NewTreeNode(
    typename TreeNode::Kind kind,
    const GraphNode* entry) {
  DCHECK(!IsFinalized());
  auto const zone = loop_tree_->zone();
  auto const tree_node = new (zone) TreeNode(zone, kind, entry);
  AssignTreeNode(entry, tree_node);
  return std::move(tree_node);
}

//////////////////////////////////////////////////////////////////////
//
// LoopTreeBuilder
//
// Constructs loop nest tree for |Graph| based on the algorithm described in:
//
// A New Algorithm for Identifying Loops in Decompilation
// Tao Wei, Jian Mao, Wei Zou, Yu Chen
// Static Analysis: Lecture Notes in Computer Science
// Volume 4634, 2007, pp 170-183
//
template <typename Graph>
class LoopTreeBuilder final : public ZoneOwner {
 public:
  using LoopTree = LoopTree<Graph>;

  explicit LoopTreeBuilder(const Graph* graph);
  ~LoopTreeBuilder() = default;

  std::unique_ptr<LoopTree> Build();

 private:
  using GraphNode = typename Graph::Derived;
  using GraphNodeSet = std::unordered_set<const GraphNode*>;
  using LoopTreeNode = typename LoopTree::TreeNode;
  using LoopTreeNodeKind = typename LoopTreeNode::Kind;

  // Node properties for loop tree building:
  //  - kind of node: root, loop header of single entry loop, loop header of
  //                  multiple entry loop.
  //  - loop header
  //  - A position in DFS spanning tree; this isn't same as RPO number.
  //  - Boolean specifies this node in path from entry node to current
  //    processing node.
  class NodeInfo : public ZoneAllocated {
   public:
    NodeInfo(const GraphNode* node, int position)
        : kind_(LoopTreeNodeKind::Root),
          loop_header_(nullptr),
          node_(node),
          position_(position) {
      DCHECK(position_);
    }
    ~NodeInfo() = delete;

    LoopTreeNodeKind kind() const { return kind_; }
    NodeInfo* loop_header() const { return loop_header_; }
    void set_loop_header(NodeInfo* node_info) { loop_header_ = node_info; }
    const GraphNode* node() const { return node_; }
    int position() const {
      DCHECK(position_);
      return position_;
    }

    bool InPath() const { return position_ > 0; }
    void MarkIrreducible() { kind_ = LoopTreeNodeKind::MultipleEntryLoop; }
    void MarkLoopHeader() { kind_ = LoopTreeNodeKind::SingleEntryLoop; }
    void RemoveFromPath() {
      DCHECK(InPath());
      position_ = 0;
    }

   private:
    LoopTreeNodeKind kind_;
    NodeInfo* loop_header_;
    const GraphNode* const node_;
    int position_;

    DISALLOW_COPY_AND_ASSIGN(NodeInfo);
  };

  NodeInfo* NodeInfoOf(const GraphNode* node) const;
  void TagLoop(NodeInfo* node_info, NodeInfo* loop_header);
  NodeInfo* Traverse(const GraphNode* node, int position);

  typename LoopTree::Editor editor_;
  const Graph* const graph_;
  std::vector<NodeInfo*> list_;
  std::unordered_map<const GraphNode*, NodeInfo*> map_;

  DISALLOW_COPY_AND_ASSIGN(LoopTreeBuilder);
};

template <typename Graph>
LoopTreeBuilder<Graph>::LoopTreeBuilder(const Graph* graph)
    : graph_(graph) {
}

template <typename Graph>
std::unique_ptr<LoopTree<Graph>> LoopTreeBuilder<Graph>::Build() {
  Traverse(graph_->first_node(), 1);
  std::unordered_map<NodeInfo*, LoopTreeNode*> tree_node_map;
  for (auto const node_info : list_) {
    if (!node_info->loop_header())
      node_info->set_loop_header(list_.front());

    if (node_info->loop_header() == node_info) {
      auto const tree_node =
          editor_.NewTreeNode(node_info->kind(), node_info->node());
      tree_node_map.insert(std::make_pair(node_info, tree_node));
      continue;
    }

    if (node_info->kind() == LoopTreeNodeKind::Root) {
      auto const it = tree_node_map.find(node_info->loop_header());
      DCHECK(it != tree_node_map.end());
      auto const loop_header_tree_node = it->second;
      editor_.AddGraphNode(loop_header_tree_node, node_info->node());
      continue;
    }

    auto const tree_node =
        editor_.NewTreeNode(node_info->kind(), node_info->node());
    tree_node_map.insert(std::make_pair(node_info, tree_node));
    auto const it = tree_node_map.find(node_info->loop_header());
    DCHECK(it != tree_node_map.end());
    auto const loop_header_tree_node = it->second;
    editor_.AddChildNode(loop_header_tree_node, tree_node);
  }
  return std::move(editor_.Finalize(graph_->first_node()));
}

template <typename Graph>
typename LoopTreeBuilder<Graph>::NodeInfo* LoopTreeBuilder<Graph>::Traverse(
    const GraphNode* node,
    int position) {
  auto const node_info = new (zone()) NodeInfo(node, position);
  map_.insert(std::make_pair(node, node_info));
  list_.push_back(node_info);
  for (auto successor : node->successors()) {
    auto const it = map_.find(successor);
    if (it == map_.end()) {
      // Case A: not visited
      auto const loop_head = Traverse(successor, position + 1);
      TagLoop(node_info, loop_head);
      continue;
    }
    auto const successor_info = it->second;
    if (successor_info->InPath()) {
      // Case B: b in path
      successor_info->MarkLoopHeader();
      TagLoop(node_info, successor_info);
      continue;
    }
    auto const loop_header = successor_info->loop_header();
    if (!loop_header) {
      // Case C: do nothing
      continue;
    }
    if (loop_header->InPath()) {
      // Case D:
      TagLoop(node_info, loop_header);
      continue;
    }
    // Case E: re-entry
    successor_info->MarkIrreducible();
    loop_header->MarkIrreducible();
    for (auto runner = loop_header; runner; runner = runner->loop_header()) {
      if (runner->InPath()) {
        TagLoop(node_info, runner);
        break;
      }
      runner->MarkIrreducible();
    }
  }
  node_info->RemoveFromPath();
  return node_info->loop_header();
}

template <typename Graph>
void LoopTreeBuilder<Graph>::TagLoop(NodeInfo* node_info,
                                     NodeInfo* loop_header) {
  if (node_info == loop_header || !loop_header)
    return;
  auto runner1 = node_info;
  auto runner2 = loop_header;
  while (auto const next = runner1->loop_header()) {
    if (next == runner2)
      return;
    if (next->position() < runner2->position()) {
      runner1->set_loop_header(runner2);
      runner1 = runner2;
      runner2 = next;
      continue;
    }
    runner1 = next;
  }
  runner1->set_loop_header(runner2);
}

template <typename Graph>
typename LoopTreeBuilder<Graph>::NodeInfo* LoopTreeBuilder<Graph>::NodeInfoOf(
    const GraphNode* node) const {
  auto const it = map_.find(node);
  DCHECK(it != map_.end());
  return it->second;
}

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_LOOP_TREE_BUILDER_H_
