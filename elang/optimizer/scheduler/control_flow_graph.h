// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_CONTROL_FLOW_GRAPH_H_
#define ELANG_OPTIMIZER_SCHEDULER_CONTROL_FLOW_GRAPH_H_

#include "base/macros.h"
#include "elang/base/graphs/graph.h"
#include "elang/base/zone_allocated.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class BasicBlock;

//////////////////////////////////////////////////////////////////////
//
// ControlFlowGraph
//
class ELANG_OPTIMIZER_EXPORT ControlFlowGraph final
    : public Graph<ControlFlowGraph, BasicBlock>,
      public ZoneAllocated {
 public:
  ControlFlowGraph();
  ~ControlFlowGraph() = delete;

 private:
  DISALLOW_COPY_AND_ASSIGN(ControlFlowGraph);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_CONTROL_FLOW_GRAPH_H_
