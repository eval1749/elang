// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ANALYSIS_DOMINATOR_TREE_BUILDER_H_
#define ELANG_HIR_ANALYSIS_DOMINATOR_TREE_BUILDER_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/base/ordered_list.h"
#include "elang/base/zone_owner.h"
#include "elang/hir/analysis/dominator_tree.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

class Function;
class Graph;
class Value;

//////////////////////////////////////////////////////////////////////
//
// DominatorTreeBuilder
//
class ELANG_HIR_EXPORT DominatorTreeBuilder final : public ZoneOwner {
 public:
  DominatorTreeBuilder(Zone* result_zone, Graph* graph);
  ~DominatorTreeBuilder();

  // |zone| for storing |DominatorTree|.
  DominatorTree* Build();

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
  DominatorTree* const dominator_tree_;
  Node* entry_node_;
  Graph* const graph_;
  Zone* const result_zone_;

  DISALLOW_COPY_AND_ASSIGN(DominatorTreeBuilder);
};

DominatorTree* ComputeDominatorTree(Zone* zone, Function* function);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ANALYSIS_DOMINATOR_TREE_BUILDER_H_
