// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/schedule_formatter.h"

#include "base/strings/stringprintf.h"
#include "elang/base/graphs/graph_sorter.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"
#include "elang/optimizer/scheduler/schedule.h"

namespace elang {
namespace optimizer {

FormattedSchedule AsFormatted(const Schedule& schedule) {
  return FormattedSchedule{&schedule};
}

std::ostream& operator<<(std::ostream& ostream,
                         const FormattedSchedule& formatted) {
  ostream << formatted.schedule->function() << std::endl;
  auto const cfg = formatted.schedule->control_flow_graph();
  auto const blocks =
      GraphSorter<ControlFlowGraph>::SortByReversePostOrder(cfg);
  auto position = 0;
  for (auto const block : blocks) {
    ostream << block << ":" << std::endl;
    {
      ostream << "  in: {";
      auto separator = "";
      for (auto const predecessor : block->predecessors()) {
        ostream << separator << predecessor;
        separator = ", ";
      }
      ostream << "}" << std::endl;
    }
    {
      ostream << "  out: {";
      auto separator = "";
      for (auto const successor : block->successors()) {
        ostream << separator << successor;
        separator = ", ";
      }
      ostream << "}" << std::endl;
    }
    for (auto const node : block->nodes()) {
      ostream << base::StringPrintf("%04d: ", position) << *node << std::endl;
      ++position;
    }
  }
  return ostream;
}

}  // namespace optimizer
}  // namespace elang
