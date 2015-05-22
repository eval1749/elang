// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

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
          << "  ranksep=\"0.8 equally\"" << std::endl
          << "  splines=true" << std::endl
          << "  node [shape=box]" << std::endl
          << std::endl;
  std::unordered_map<Node*, Node*> end_to_start_map;
  Node* start_node = nullptr;
  auto& schedule = *wrapper.schedule;
  for (auto const node : schedule.nodes()) {
    if (node->IsBlockStart()) {
      ostream << " node" << node->id() << " [label=\"";
      start_node = node;
    } else {
      DCHECK(start_node);
    }
    ostream << PrefixOf(node) << node->id() << "=" << node->mnemonic();
    for (auto const input : node->inputs())
      ostream << " " << PrefixOf(input) << input->id();
    ostream << "\\l";
    if (!node->IsBlockEnd())
      continue;
    ostream << "\"]" << std::endl;
    end_to_start_map.insert(std::make_pair(node, start_node));
    start_node = nullptr;
  }

  ostream << std::endl << "  // Edges" << std::endl;
  for (auto const node : schedule.nodes()) {
    if (!node->IsBlockStart())
      continue;
    auto position = 0;
    auto const to = node;
    for (auto const input : node->inputs()) {
      auto const it = end_to_start_map.find(input);
      DCHECK(it != end_to_start_map.end());
      auto const from = it->second;
      auto const is_back_edge = position && node->opcode() == Opcode::Loop;
      auto const port = is_back_edge ? ":w" : "";
      ostream << "  node" << from->id() << port << " -> node" << to->id()
              << port;
      if (is_back_edge)
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
