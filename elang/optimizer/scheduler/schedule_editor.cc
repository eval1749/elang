// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/schedule_editor.h"

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree_builder.h"
#include "elang/base/analysis/loop_tree_builder.h"
#include "elang/optimizer/nodes.h"
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

int ScheduleEditor::User::LoopDepthOf(BasicBlock* block) const {
  return editor_.LoopDepthOf(block);
}

//////////////////////////////////////////////////////////////////////
//
// ScheduleEditor
//
ScheduleEditor::ScheduleEditor(Schedule* schedule)
    : ZoneUser(schedule->zone()),
      schedule_(*schedule) {
}

ScheduleEditor::~ScheduleEditor() {
}

ControlFlowGraph* ScheduleEditor::control_flow_graph() const {
  return schedule_.control_flow_graph();
}

Function* ScheduleEditor::function() const {
  return schedule_.function();
}

void ScheduleEditor::AppendNode(BasicBlock* block, Node* node) {
  if (block->nodes_.empty()) {
    block->nodes_.push_back(node);
    return;
  }
  auto const last = block->nodes_.back();
  if (!last->IsBlockEnd()) {
    block->nodes_.push_back(node);
    return;
  }
  block->nodes_.back() = node;
  block->nodes_.push_back(last);
}

BasicBlock* ScheduleEditor::BlockOf(Node* node) const {
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

BasicBlock* ScheduleEditor::DominatorOf(BasicBlock* block) const {
  return dominator_tree_->TreeNodeOf(block)->parent()->value();
}

void ScheduleEditor::FinishControlFlowGraph() {
  DCHECK(!dominator_tree_);
  DCHECK(!loop_tree_);
  auto const cfg = schedule_.control_flow_graph();
  dominator_tree_ = DominatorTreeBuilder<ControlFlowGraph>(cfg).Build();
  loop_tree_ = LoopTreeBuilder<ControlFlowGraph>(cfg).Build();
}

int ScheduleEditor::LoopDepthOf(BasicBlock* block) const {
  return loop_tree_->NodeOf(block)->depth();
}

BasicBlock* ScheduleEditor::MapToBlock(Node* start_node) {
  DCHECK(!dominator_tree_);
  DCHECK(!loop_tree_);
  DCHECK(start_node->IsBlockStart()) << *start_node;
  auto const it = block_map_.find(start_node);
  if (it != block_map_.end())
    return it->second;
  auto const block = new (zone()) BasicBlock(zone());
  block_map_.insert(std::make_pair(start_node, block));
  return block;
}

void ScheduleEditor::SetBlockOf(Node* node, BasicBlock* block) {
  DCHECK(node);
  DCHECK(block);
  DCHECK(!block_map_.count(node));
  block_map_.insert(std::make_pair(node, block));
}

}  // namespace optimizer
}  // namespace elang
