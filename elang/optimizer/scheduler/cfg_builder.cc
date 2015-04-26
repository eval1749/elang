// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/cfg_builder.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/schedule_editor.h"
#include "elang/optimizer/scheduler/scheduler.h"

namespace elang {
namespace optimizer {

namespace {
bool Has(const ZoneVector<BasicBlock*>& blocks, BasicBlock* block) {
  return std::find(blocks.begin(), blocks.end(), block) != blocks.end();
}
}  // namespace

CfgBuilder::CfgBuilder(ScheduleEditor* editor)
    : block_(nullptr),
      cfg_editor_(editor->control_flow_graph()),
      editor_(*editor) {
}

CfgBuilder::~CfgBuilder() {
  DCHECK(!block_) << *block_;
  DCHECK(BlockOf(editor_.function()->entry_node()));
  DCHECK(BlockOf(editor_.function()->exit_node()));
}

BasicBlock* CfgBuilder::BlockOf(Node* node) {
  return editor_.BlockOf(node);
}

void CfgBuilder::EndBlock(Node* end_node) {
  DCHECK(end_node->IsBlockEnd()) << *end_node;
  DCHECK(block_);
  editor_.SetBlockOf(end_node, block_);
  for (auto edge : end_node->use_edges()) {
    auto const successor = edge->from()->as<Control>();
    if (!successor)
      continue;
    cfg_editor_.AddEdge(block_, editor_.MapToBlock(successor));
  }
  block_ = nullptr;
}

// The entry point.
// reverse post order traversal on control edge.
void CfgBuilder::Run() {
  DepthFirstTraversal<OnControlEdge, const Function> walker;
  auto const function = editor_.function();
  walker.Traverse(function, this);
  editor_.DidBuildControlFlowGraph();
}

void CfgBuilder::StartBlock(Node* start_node) {
  DCHECK(start_node->IsBlockStart());
  DCHECK(!block_) << *block_;
  block_ = editor_.MapToBlock(start_node);
  cfg_editor_.AppendNode(block_);
  auto const phi_owner = start_node->as<PhiOwnerNode>();
  if (!phi_owner)
    return;
  if (auto const effect_phi = phi_owner->effect_phi())
    editor_.SetBlockOf(effect_phi, block_);
  for (auto const phi : phi_owner->phi_nodes())
    editor_.SetBlockOf(phi, block_);
}

// NodeVisitor protocol
void CfgBuilder::DoDefaultVisit(Node* node) {
  DCHECK(node->IsControl());
  if (node->IsBlockStart())
    return StartBlock(node);
  DCHECK(block_);
  if (node->IsBlockEnd())
    return EndBlock(node);
  editor_.SetBlockOf(node, block_);
}

}  // namespace optimizer
}  // namespace elang
