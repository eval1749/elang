// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/analysis/dominator_tree.h"

#include "elang/hir/values.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// DominatorTree::Node
//
DominatorTree::Node::Node(Zone* zone, Value* value)
    : children_(zone),
      depth_(0),
      frontiers_(zone),
      parent_(nullptr),
      value_(value) {
}

//////////////////////////////////////////////////////////////////////
//
// DominatorTree::Node
//
DominatorTree::DominatorTree() : node_map_(zone()) {
}

DominatorTree::~DominatorTree() {
}

DominatorTree::Node* DominatorTree::node_of(Value* value) const {
  auto const it = node_map_.find(value);
  DCHECK(it != node_map_.end());
  return it->second;
}

bool DominatorTree::Dominates(Value* dominator, Value* dominatee) const {
  auto const dominator_node = node_of(dominator);
  auto const dominatee_node = node_of(dominatee);
  for (auto runner = dominatee_node; runner; runner = runner->parent()) {
    if (runner == dominator_node)
      return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& ostream,
                         const DominatorTree::Node& node) {
  ostream << "{value: " << *node.value()
          << ", parent: " << *node.parent()->value() << " children: [";
  auto separator = "";
  for (auto const child : node.children()) {
    ostream << separator << child->value();
    separator = ", ";
  }
  ostream << "], frontiers: [";
  separator = "";
  for (auto const frontier : node.frontiers()) {
    ostream << separator << frontier->value();
    separator = ", ";
  }
  return ostream << "]}";
}

std::ostream& operator<<(std::ostream& ostream,
                         const DominatorTree::Node* node) {
  if (!node)
    return ostream << "(null)";
  return ostream << *node;
}

}  // namespace hir
}  // namespace elang
