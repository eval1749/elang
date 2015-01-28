// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/hir/analysis/dominator_tree_builder.h"

#include "base/logging.h"
#include "elang/base/zone_user.h"
#include "elang/hir/analysis/dominator_tree.h"
#include "elang/hir/analysis/graph.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// DominatorTreeBuilder
//
DominatorTreeBuilder::DominatorTreeBuilder(Zone* result_zone, Graph* graph)
    : dfs_list_(graph->ReversePostOrderList()),
      dominator_tree_(new (result_zone) DominatorTree(result_zone)),
      entry_node_(nullptr),
      graph_(graph),
      result_zone_(result_zone) {
}

DominatorTreeBuilder::~DominatorTreeBuilder() {
}

int DominatorTreeBuilder::dfs_position_of(Node* node) const {
  return dfs_list_.position_of(node->value());
}

DominatorTree::Node* DominatorTreeBuilder::node_of(Value* value) const {
  return dominator_tree_->node_of(value);
}

DominatorTree* DominatorTreeBuilder::Build() {
  for (auto const value : dfs_list_) {
    dominator_tree_->node_map_[value] =
        new (result_zone_) Node(result_zone_, value);
  }
  entry_node_ = dominator_tree_->node_of(graph_->entry());
  // Set sentinel
  entry_node_->parent_ = entry_node_;
  entry_node_->depth_ = 1;
  ComputeParentForAll();
  entry_node_->parent_ = nullptr;
  ComputeChildren();
  ComputeFrontiers();
  return dominator_tree_;
}

void DominatorTreeBuilder::ComputeChildren() {
  for (auto value : dfs_list_) {
    auto const node = node_of(value);
    auto const parent = node->parent();
    if (!parent) {
      DCHECK_EQ(entry_node_, node);
      continue;
    }
    parent->children_.push_back(node);
  }
}

//  Loop over all basic block which has more than one predecessors.
void DominatorTreeBuilder::ComputeFrontiers() {
  for (auto const value : dfs_list_) {
    if (!graph_->HasMoreThanOnePredecessors(value))
      continue;
    auto const node = dominator_tree_->node_of(value);
    for (auto const predecessor_value : graph_->PredecessorsOf(value)) {
      auto const predecessor = dominator_tree_->node_of(predecessor_value);
      for (auto runner = predecessor; runner != node->parent();
           runner = runner->parent()) {
        auto const it = std::find(runner->frontiers_.begin(),
                                  runner->frontiers_.end(), node);
        if (it == runner->frontiers_.end())
          runner->frontiers_.push_back(node);
      }
    }
  }
}

void DominatorTreeBuilder::ComputeParentForAll() {
  auto changed = true;
  while (changed) {
    changed = false;
    for (auto const value : dfs_list_) {
      auto const node = dominator_tree_->node_of(value);
      if (ComputeParentForNode(node))
        changed = true;
    }
  }
}

bool DominatorTreeBuilder::ComputeParentForNode(Node* node) {
  for (auto const parent_value : graph_->PredecessorsOf(node->value())) {
    auto candidate = node_of(parent_value);
    if (!candidate->parent())
      continue;

    for (auto const predecessor_value : graph_->PredecessorsOf(node->value())) {
      auto const predecessor = node_of(predecessor_value);
      if (candidate != predecessor && predecessor->parent())
        candidate = Intersect(candidate, predecessor);
    }

    if (node->parent() != candidate) {
      node->parent_ = candidate;
      node->depth_ = candidate->depth() + 1;
      return true;
    }
  }
  return false;
}

DominatorTree::Node* DominatorTreeBuilder::Intersect(Node* finger1,
                                                     Node* finger2) {
  while (finger1 != finger2) {
    while (dfs_position_of(finger1) > dfs_position_of(finger2))
      finger1 = finger1->parent();
    while (dfs_position_of(finger2) > dfs_position_of(finger1))
      finger2 = finger2->parent();
  }
  return finger1;
}

DominatorTree* ComputeDominatorTree(Zone* zone, Function* function) {
  ControlFlowGraph cfg(function);
  return DominatorTreeBuilder(zone, &cfg).Build();
}

}  // namespace hir
}  // namespace elang
