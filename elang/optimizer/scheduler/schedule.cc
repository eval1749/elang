// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/schedule.h"

#include "elang/optimizer/nodes.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"
#include "elang/optimizer/scheduler/formatted_schedule.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Schedule
//
Schedule::Schedule(Function* function) : function_(function), nodes_(zone()) {
}

Schedule::~Schedule() {
}

std::ostream& operator<<(std::ostream& ostream, const Schedule& schedule) {
  return ostream << AsFormatted(schedule);
}

}  // namespace optimizer
}  // namespace elang
