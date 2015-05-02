// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/visual_schedule.h"

#include "base/strings/stringprintf.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"
#include "elang/optimizer/scheduler/schedule.h"

namespace elang {
namespace optimizer {

namespace {
base::StringPiece PrefixOf(Node* node) {
  if (node->IsControl())
    return "%c";
  if (node->IsEffect())
    return "%e";
  if (node->IsTuple())
    return "%t";
  return "%r";
}
}  // namespace

std::ostream& operator<<(std::ostream& ostream, const VisualSchedule& wrapper) {
  ostream << "digraph IR {" << std::endl
          << "  concentrate=false" << std::endl
          << "  node [fontname=monospace fontsize=10]" << std::endl
          << "  overlap=false" << std::endl
          << "  rankdir=\"TB\"" << std::endl
          << "  ranksep=\"0.2 equally\"" << std::endl
          << "  splines=true" << std::endl
          << std::endl;

  auto& schedule = *wrapper.schedule;
  auto last = static_cast<const Node*>(nullptr);
  for (auto const node : schedule.nodes()) {
    if (node->IsBlockStart()) {
      ostream << "  subgraph cluster_" << node->id() << " {" << std::endl;
      last = nullptr;
    }
    ostream << "    node" << node->id() << " [shape=record "
            << "label=\"{{" << PrefixOf(node) << node->id() << "="
            << node->mnemonic();
    for (auto const input : node->inputs())
      ostream << "|" << PrefixOf(input) << input->id();
    ostream << "}}\"]" << std::endl;
    if (last) {
      ostream << "    node" << last->id() << " -> node" << node->id()
              << std::endl;
    }
    last = node;
    if (node->IsBlockEnd())
      ostream << "  }" << std::endl;
  }

  ostream << std::endl
          << "  // Edges" << std::endl;
  for (auto const node : schedule.nodes()) {
    if (!node->IsBlockStart())
      continue;
    auto position = 0;
    auto const to = node->id();
    for (auto const input : node->inputs()) {
      auto const from = input->id();
      ostream << "  node" << from << " -> node" << to;
      if (position && node->opcode() == Opcode::Loop)
        ostream << " [color=red constraint=false]";
      ostream << std::endl;
      ++position;
    }
  }
  return ostream << "}" << std::endl;
}

VisualSchedule AsVisual(const Schedule& schedule) {
  return VisualSchedule{&schedule};
}

}  // namespace optimizer
}  // namespace elang
