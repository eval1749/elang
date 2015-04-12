// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/basic_block.h"

#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
BasicBlock::BasicBlock(Zone* zone, Node* start_node)
    : nodes_(zone), predecessors_(zone), successors_(zone) {
  DCHECK(start_node->IsBlockStart());
  nodes_.push_back(start_node);
}

}  // namespace optimizer
}  // namespace elang
