// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ANALYSIS_DOMINATOR_TREE_BUILDER_H_
#define ELANG_HIR_ANALYSIS_DOMINATOR_TREE_BUILDER_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "elang/base/ordered_list.h"
#include "elang/base/zone_owner.h"
#include "elang/hir/analysis/dominator_tree.h"

namespace elang {
namespace hir {

class Function;
class Graph;
class Value;

//////////////////////////////////////////////////////////////////////
//
// DominatorTreeBuilder
//
class DominatorTreeBuilder final : public ZoneOwner {
 public:
  explicit DominatorTreeBuilder(Graph* graph);
  ~DominatorTreeBuilder();

  // |zone| for storing |DominatorTree|.
  std::unique_ptr<DominatorTree> Build();

 private:
  typedef DominatorTree::Node Node;

  int dfs_position_of(Node* node) const;
  Node* node_of(Value* value) const;

  void ComputeChildren();
  void ComputeFrontiers();
  void ComputeParentForAll();
  bool ComputeParentForNode(Node* node);
  Node* Intersect(Node* node1, Node* node2);

  OrderedList<Value*> dfs_list_;
  std::unique_ptr<DominatorTree> dominator_tree_;
  Node* entry_node_;
  Graph* const graph_;

  DISALLOW_COPY_AND_ASSIGN(DominatorTreeBuilder);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ANALYSIS_DOMINATOR_TREE_BUILDER_H_
