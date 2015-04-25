// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_BASIC_BLOCK_H_
#define ELANG_OPTIMIZER_SCHEDULER_BASIC_BLOCK_H_

#include <ostream>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"

namespace elang {
namespace optimizer {

class Node;
class ScheduleEditor;

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
class ELANG_OPTIMIZER_EXPORT BasicBlock final
    : public ControlFlowGraph::GraphNodeBase,
      public ZoneAllocated {
 public:
  using Node = ::elang::optimizer::Node;

  ~BasicBlock() = delete;

  const ZoneVector<Node*>& nodes() const { return nodes_; }

 private:
  friend class ScheduleEditor;

  explicit BasicBlock(Zone* zone);

  ZoneVector<Node*> nodes_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const BasicBlock* block);
ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const BasicBlock& block);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_BASIC_BLOCK_H_
