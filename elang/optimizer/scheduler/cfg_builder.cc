// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/cfg_builder.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/optimizer/scheduler/scheduler.h"

namespace elang {
namespace optimizer {

namespace {
bool Has(const ZoneVector<BasicBlock*>& blocks, BasicBlock* block) {
  return std::find(blocks.begin(), blocks.end(), block) != blocks.end();
}
}  // namespace

CfgBuilder::CfgBuilder(Schedule* schedule)
    : ZoneUser(schedule->zone()),
      block_end_node_(nullptr),
      schedule_(*schedule) {
}

CfgBuilder::~CfgBuilder() {
  DCHECK(!block_end_node_);
  DCHECK(BlockOf(schedule_.function()->entry_node()));
  DCHECK(BlockOf(schedule_.function()->exit_node()));
}

void CfgBuilder::AddCfgEdge(BasicBlock* from, BasicBlock* to) {
  DCHECK(!Has(from->successors_, to));
  DCHECK(!Has(to->predecessors_, from));
  from->successors_.push_back(to);
  to->predecessors_.push_back(from);
}

BasicBlock* CfgBuilder::BlockOf(Node* node) {
  return schedule_.BlockOf(node);
}

void CfgBuilder::EndBlock(Node* node) {
  DCHECK(node->IsBlockEnd()) << *node;
  DCHECK(!block_end_node_) << *node;
  block_end_node_ = node;
}

BasicBlock* CfgBuilder::NewBasicBlock(Node* start) {
  auto const it = schedule_.block_map_.find(start);
  if (it != schedule_.block_map_.end())
    return it->second;
  auto const block = new (zone()) BasicBlock(zone(), start);
  schedule_.block_map_[start] = block;
  return block;
}

// The entry point
void CfgBuilder::Run() {
  DepthFirstTraversal<OnControlEdge, const Function> walker;
  auto const function = schedule_.function();
  walker.Traverse(function, this);
}

void CfgBuilder::StartBlock(Node* block_start_node) {
  DCHECK(block_start_node->IsBlockStart()) << *block_start_node;
  DCHECK(block_end_node_) << *block_start_node;
  auto const block = NewBasicBlock(block_start_node);
  DCHECK(!block_end_node_) << *block_start_node;
  block->nodes_.push_back(block_end_node_);
  for (auto edge : block_end_node_->use_edges()) {
    auto const successor = edge->to()->as<Control>();
    if (!successor)
      continue;
    AddCfgEdge(block, NewBasicBlock(successor));
  }
  block_end_node_ = nullptr;
}

// NodeVisitor protocol
void CfgBuilder::DoDefaultVisit(Node* node) {
  if (node->IsBlockStart())
    return StartBlock(node);
  if (node->IsBlockEnd())
    return EndBlock(node);
}

}  // namespace optimizer
}  // namespace elang
