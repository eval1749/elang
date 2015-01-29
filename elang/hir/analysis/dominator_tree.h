// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ANALYSIS_DOMINATOR_TREE_H_
#define ELANG_HIR_ANALYSIS_DOMINATOR_TREE_H_

#include <ostream>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

class DominatorTreeBuilder;
class Value;

//////////////////////////////////////////////////////////////////////
//
// DominatorTree
//
class ELANG_HIR_EXPORT DominatorTree final : public ZoneOwner {
 public:
  class Node : public ZoneAllocated {
   public:
    ~Node() = delete;

    const ZoneVector<Node*>& children() const { return children_; }
    int depth() const { return depth_; }
    const ZoneVector<Node*>& frontiers() const { return frontiers_; }
    Node* parent() const { return parent_; }
    Value* value() const { return value_; }

   private:
    friend class DominatorTreeBuilder;

    Node(Zone* zone, Value* value);

    ZoneVector<Node*> children_;
    int depth_;
    ZoneVector<Node*> frontiers_;
    Node* parent_;
    Value* const value_;

    DISALLOW_COPY_AND_ASSIGN(Node);
  };

  ~DominatorTree();

  Node* node_of(Value* value) const;

 private:
  friend class DominatorTreeBuilder;

  DominatorTree();

  ZoneUnorderedMap<Value*, Node*> node_map_;

  DISALLOW_COPY_AND_ASSIGN(DominatorTree);
};

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const DominatorTree::Node& node);

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const DominatorTree::Node* node);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ANALYSIS_DOMINATOR_TREE_H_
