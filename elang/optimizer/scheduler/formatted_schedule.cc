// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/formatted_schedule.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
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
  struct Local {
    static Control* ControlUserOf(const Node* node) {
      DCHECK(node->IsControl());
      for (auto const edge : node->use_edges()) {
        if (auto const control = edge->from()->as<Control>())
          return control;
      }
      NOTREACHED();
      return nullptr;
    }
    static Node* LastNodeOf(const Node* node) {
      DCHECK(node->IsBlockStart());
      for (auto runner = const_cast<Node*>(node); runner;
           runner = ControlUserOf(runner)) {
        if (runner->IsBlockEnd())
          return runner;
      }
      NOTREACHED();
      return nullptr;
    }
    static Node* StartNodeOf(const Node* node) {
      DCHECK(!node->IsBlockStart());
      for (auto runner = const_cast<Node*>(node); runner;
           runner = runner->input(0)) {
        if (runner->IsBlockStart())
          return runner;
      }
      NOTREACHED();
      return nullptr;
    }
  };
  auto& schedule = *formatted.schedule;
  ostream << schedule.function() << std::endl;
  auto position = 0;
  for (auto const node : schedule.nodes()) {
    if (node->IsBlockStart()) {
      ostream << "block" << node->id() << ":" << std::endl;
      {
        ostream << "  In:   {";
        auto separator = "";
        for (auto const input : node->inputs()) {
          ostream << separator << "block" << Local::StartNodeOf(input)->id();
          separator = ", ";
        }
        ostream << "}" << std::endl;
      }
      {
        ostream << "  Out:  {";
        auto separator = "";
        for (auto const use_edge : Local::LastNodeOf(node)->use_edges()) {
          ostream << separator << "block" << use_edge->from()->id();
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
