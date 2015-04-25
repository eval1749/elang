// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/schedule.h"

#include "elang/optimizer/nodes.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Schedule
//
Schedule::Schedule(Function* function)
    : control_flow_graph_(new (zone()) ControlFlowGraph()),
      function_(function) {
}

Schedule::~Schedule() {
}

}  // namespace optimizer
}  // namespace elang
