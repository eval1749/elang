// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_
#define ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_

#include "base/macros.h"
#include "elang/base/zone_owner.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class ControlFlowGraph;
class Function;
class Node;
class ScheduleEdtior;

//////////////////////////////////////////////////////////////////////
//
// Schedule
//
class ELANG_OPTIMIZER_EXPORT Schedule final : public ZoneOwner {
 public:
  explicit Schedule(Function* function);
  ~Schedule();

  ControlFlowGraph* control_flow_graph() const { return control_flow_graph_; }
  Function* function() const { return function_; }

 private:
  friend class ScheduleEditor;

  ControlFlowGraph* const control_flow_graph_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Schedule);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_
