// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/formatted_schedule.h"

#include "base/strings/stringprintf.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/optimizer/scheduler/schedule_editor.h"

namespace elang {
namespace optimizer {

FormattedSchedule AsFormatted(const Schedule& schedule) {
  return FormattedSchedule{&schedule};
}

std::ostream& operator<<(std::ostream& ostream,
                         const FormattedSchedule& formatted) {
  auto& schedule = *formatted.schedule;
  ostream << schedule.function() << std::endl;
  auto position = 0;
  for (auto const node : schedule.nodes()) {
    if (node->IsBlockStart())
      ostream << "block" << node->id() << ":" << std::endl;
    ostream << base::StringPrintf("%04d: ", position) << *node << std::endl;
    ++position;
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const ScheduleEditor& editor) {
  auto& schedule = editor.schedule();
  ostream << schedule.function() << std::endl;
  auto position = 0;
  for (auto const node : schedule.nodes()) {
    if (node->IsBlockStart()) {
      auto const block = editor.BlockOf(node);
      ostream << block << ":" << std::endl;
      {
        ostream << "  In:   {";
        auto separator = "";
        for (auto const control : node->inputs()) {
          ostream << separator << editor.BlockOf(control);
          separator = ", ";
        }
        ostream << "}" << std::endl;
      }
      {
        ostream << "  Out:  {";
        auto separator = "";
        for (auto const use_edge : block->last_node()->use_edges()) {
          ostream << separator << editor.BlockOf(use_edge->from());
          separator = ", ";
        }
        ostream << "}" << std::endl;
      }
    }
    ostream << base::StringPrintf("  %04d: ", position) << *node << std::endl;
    ++position;
  }
  return ostream;
}

}  // namespace optimizer
}  // namespace elang
