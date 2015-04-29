// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/schedule_editor.h"

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree_builder.h"
#include "elang/base/analysis/loop_tree_builder.h"
#include "elang/base/graphs/graph_sorter.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"
#include "elang/optimizer/scheduler/schedule.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// ScheduleEditor::User
//
ScheduleEditor::User::User(ScheduleEditor* editor) : editor_(*editor) {
}

ScheduleEditor::User::~User() {
}

BasicBlock* ScheduleEditor::User::BlockOf(Node* node) const {
  return editor_.BlockOf(node);
}

BasicBlock* ScheduleEditor::User::CommonAncestorOf(BasicBlock* block1,
                                                   BasicBlock* block2) const {
  return editor_.CommonAncestorOf(block1, block2);
}

int ScheduleEditor::User::DepthOf(BasicBlock* block) const {
  return editor_.DepthOf(block);
}

BasicBlock* ScheduleEditor::User::DominatorOf(BasicBlock* block) const {
  return editor_.DominatorOf(block);
}

int ScheduleEditor::User::LoopDepthOf(const BasicBlock* block) const {
  return editor_.LoopDepthOf(block);
}

BasicBlock* ScheduleEditor::User::LoopHeaderOf(const BasicBlock* block) const {
  return editor_.LoopHeaderOf(block);
}

//////////////////////////////////////////////////////////////////////
//
// ScheduleEditor
//
ScheduleEditor::ScheduleEditor(Schedule* schedule)
    : ZoneUser(schedule->zone()),
      control_flow_graph_(new (zone()) ControlFlowGraph()),
      schedule_(*schedule) {
}

ScheduleEditor::~ScheduleEditor() {
}

Function* ScheduleEditor::function() const {
  return schedule_.function();
}

void ScheduleEditor::AppendNode(BasicBlock* block, Node* node) {
  DCHECK(!block->nodes_.empty());
  if (node->is<PhiNode>() || node->is<EffectPhiNode>())
    return;
  if (node->IsBlockStart()) {
    DCHECK_EQ(block->nodes_.front(), node);
    return;
  }
  auto const last_node = block->nodes_.back();
  if (!last_node->IsBlockEnd()) {
    block->nodes_.push_back(node);
    return;
  }
  block->nodes_.back() = node;
  block->nodes_.push_back(last_node);
}

BasicBlock* ScheduleEditor::BlockOf(Node* node) const {
  DCHECK(!node->IsLiteral());
  auto const it = block_map_.find(node);
  return it == block_map_.end() ? nullptr : it->second;
}

BasicBlock* ScheduleEditor::CommonAncestorOf(BasicBlock* block1,
                                             BasicBlock* block2) {
  return dominator_tree_->CommonAncestorOf(block1, block2);
}

int ScheduleEditor::DepthOf(BasicBlock* block) const {
  return dominator_tree_->TreeNodeOf(block)->depth();
}

void ScheduleEditor::DidBuildControlFlowGraph() {
  DCHECK(!dominator_tree_);
  DCHECK(!loop_tree_);
  dominator_tree_ =
      DominatorTreeBuilder<ControlFlowGraph>(control_flow_graph_).Build();
  loop_tree_ = LoopTreeBuilder<ControlFlowGraph>(control_flow_graph_).Build();
}

void ScheduleEditor::DidPlaceNodes(const std::vector<Node*>& nodes) {
  schedule_.nodes_.reserve(nodes.size());
  schedule_.nodes_.insert(schedule_.nodes_.end(), nodes.begin(), nodes.end());
}

BasicBlock* ScheduleEditor::DominatorOf(BasicBlock* block) const {
  return dominator_tree_->TreeNodeOf(block)->parent()->value();
}

int ScheduleEditor::LoopDepthOf(const BasicBlock* block) const {
  return loop_tree_->NodeOf(const_cast<BasicBlock*>(block))->depth();
}

BasicBlock* ScheduleEditor::LoopHeaderOf(const BasicBlock* block) const {
  return loop_tree_->NodeOf(const_cast<BasicBlock*>(block))->entry();
}

BasicBlock* ScheduleEditor::MapToBlock(Node* start_node) {
  DCHECK(!dominator_tree_);
  DCHECK(!loop_tree_);
  DCHECK(start_node->IsBlockStart()) << *start_node;
  auto const it = block_map_.find(start_node);
  if (it != block_map_.end())
    return it->second;
  auto const block = new (zone()) BasicBlock(zone());
  block->nodes_.push_back(start_node);
  block_map_.insert(std::make_pair(start_node, block));
  return block;
}

void ScheduleEditor::SetBlockOf(Node* node, BasicBlock* block) {
  DCHECK(node);
  DCHECK(block) << *node;
  block_map_[node] = block;
}

}  // namespace optimizer
}  // namespace elang
