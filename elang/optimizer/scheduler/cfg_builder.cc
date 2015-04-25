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
    : block_(nullptr), editor_(*editor) {
}

CfgBuilder::~CfgBuilder() {
  DCHECK(!block_) << *block_;
  DCHECK(BlockOf(editor_.function()->entry_node()));
  DCHECK(BlockOf(editor_.function()->exit_node()));
}

BasicBlock* CfgBuilder::BlockOf(Node* node) {
  return editor_.BlockOf(node);
}

void CfgBuilder::EndBlock(Node* node) {
  DCHECK(block_);
  editor_.EndBlock(block_, node);
  block_ = nullptr;
}

// The entry point.
// reverse post order traversal on control edge.
void CfgBuilder::Run() {
  DepthFirstTraversal<OnControlEdge, const Function> walker;
  auto const function = editor_.function();
  walker.Traverse(function, this);
  editor_.FinishControlFlowGraph();
}

void CfgBuilder::StartBlock(Node* start_node) {
  DCHECK(!block_) << *block_;
  block_ = editor_.StartBlock(start_node);
}

// NodeVisitor protocol
void CfgBuilder::DoDefaultVisit(Node* node) {
  if (node->IsBlockStart())
    return StartBlock(node);
  DCHECK(block_);
  if (node->IsBlockEnd())
    return EndBlock(node);
  if (auto const phi = node->as<PhiNode>())
    return editor_.SetBlockOf(node, editor_.StartBlock(phi->owner()));
  if (auto const phi = node->as<EffectPhiNode>())
    return editor_.SetBlockOf(node, editor_.StartBlock(phi->owner()));
}

}  // namespace optimizer
}  // namespace elang
