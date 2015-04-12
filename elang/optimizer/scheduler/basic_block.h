// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_BASIC_BLOCK_H_
#define ELANG_OPTIMIZER_SCHEDULER_BASIC_BLOCK_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"

namespace elang {
namespace optimizer {

class Node;

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
class BasicBlock final : public ZoneAllocated {
 public:
  ~BasicBlock() = delete;

 private:
  friend class CfgBuilder;

  BasicBlock(Zone* zone, Node* start_node);

  ZoneVector<Node*> nodes_;
  ZoneVector<BasicBlock*> predecessors_;
  ZoneVector<BasicBlock*> successors_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_BASIC_BLOCK_H_
