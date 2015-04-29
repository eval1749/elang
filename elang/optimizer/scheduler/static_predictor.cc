// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "elang/optimizer/scheduler/static_predictor.h"

#include "elang/base/graphs/graph_sorter.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/scheduler/basic_block.h"

namespace elang {
namespace optimizer {

namespace {

bool IsZero(const Node* node) {
  if (auto const lit = node->as<Int32Node>())
    return !lit->data();
  if (auto const lit = node->as<UInt32Node>())
    return !lit->data();
  if (auto const lit = node->as<Int64Node>())
    return !lit->data();
  if (auto const lit = node->as<UInt64Node>())
    return !lit->data();
  if (auto const lit = node->as<Int16Node>())
    return !lit->data();
  if (auto const lit = node->as<UInt16Node>())
    return !lit->data();
  if (auto const lit = node->as<Int8Node>())
    return !lit->data();
  if (auto const lit = node->as<UInt8Node>())
    return !lit->data();
  return false;
}

// Returns probability of taken branch.
// TODO(eval1749) character equality heuristics
// TODO(eval1749) Bit test heuristics
double EstimateByCondition(const Node* node) {
  if (auto const icmp = node->as<IntCmpNode>()) {
    if (icmp->input(0)->is<NullNode>())
      return icmp->condition() == IntCondition::Equal ? 0.1 : 0.9;
    if (icmp->input(0)->output_type()->is<PointerType>())
      return icmp->condition() == IntCondition::UnsignedLessThan ? 0.8 : 0.2;
    if (!IsZero(node->input(1)))
      return 0.5;
    switch (icmp->condition()) {
      case IntCondition::Equal:
      case IntCondition::UnsignedLessThanOrEqual:
        return 0.1;
      case IntCondition::NotEqual:
        return 0.9;
      case IntCondition::SignedLessThan:
      case IntCondition::SignedLessThanOrEqual:
        return 0.2;
      case IntCondition::SignedGreaterThan:
      case IntCondition::SignedGreaterThanOrEqual:
        return 0.8;
      case IntCondition::UnsignedGreaterThan:
      case IntCondition::UnsignedGreaterThanOrEqual:
        return 0.9;
      case IntCondition::UnsignedLessThan:
        return 0.0;
    }
    NOTREACHED() << *icmp;
    return 0.5;
  }
  if (auto const fcmp = node->as<FloatCmpNode>()) {
    return fcmp->condition() == FloatCondition::OrderedEqual ||
                   fcmp->condition() == FloatCondition::OrderedNotEqual
               ? 0.1
               : 0.5;
  }
  return 0.5;
}

double EstimateBySuccessor(const BasicBlock* block) {
  auto const last_node = block->last_node();
  if (last_node->opcode() == Opcode::Throw ||
      last_node->opcode() == Opcode::Unreachable) {
    return 0;
  }
  return 1;
}

Node* FalseTargetOf(const BasicBlock* block) {
  auto const last_node = block->last_node();
  DCHECK_EQ(last_node->opcode(), Opcode::If);
  for (auto const use_edge : last_node->use_edges()) {
    if (use_edge->from()->opcode() == Opcode::IfFalse)
      return use_edge->from();
  }
  NOTREACHED() << *last_node;
  return nullptr;
}

Node* JumpTargetOf(const BasicBlock* block) {
  auto const last_node = block->last_node();
  if (last_node->opcode() != Opcode::Jump)
    return nullptr;
  return last_node->use_edges().begin()->from();
}

Node* TrueTargetOf(const BasicBlock* block) {
  auto const last_node = block->last_node();
  DCHECK_EQ(last_node->opcode(), Opcode::If);
  for (auto const use_edge : last_node->use_edges()) {
    if (use_edge->from()->opcode() == Opcode::IfTrue)
      return use_edge->from();
  }
  NOTREACHED() << *last_node;
  return nullptr;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EdgeFrequencyMap::Editor
//
EdgeFrequencyMap::Editor::Editor() : edge_map_(new EdgeFrequencyMap()) {
}

EdgeFrequencyMap::Editor::~Editor() {
  DCHECK(!edge_map_);
}

void EdgeFrequencyMap::Editor::Add(const BasicBlock* from,
                                   const BasicBlock* to,
                                   double value) {
  DCHECK_GE(value, 0);
  auto const key = std::make_pair(from, to);
  DCHECK(!edge_map_->map_.count(key)) << from << "->" << to;
  edge_map_->map_.insert(std::make_pair(key, value));
}

std::unique_ptr<EdgeFrequencyMap> EdgeFrequencyMap::Editor::Finish() {
  DCHECK(edge_map_);
  return std::move(edge_map_);
}

double EdgeFrequencyMap::Editor::FrequencyOf(const BasicBlock* from,
                                             const BasicBlock* to) const {
  return edge_map_->FrequencyOf(from, to);
}

bool EdgeFrequencyMap::Editor::Has(const BasicBlock* from,
                                   const BasicBlock* to) const {
  auto const it = edge_map_->map_.find(std::make_pair(from, to));
  return it != edge_map_->map_.end();
}

//////////////////////////////////////////////////////////////////////
//
// EdgeFrequencyMap
//
EdgeFrequencyMap::EdgeFrequencyMap() {
}

EdgeFrequencyMap::~EdgeFrequencyMap() {
}

double EdgeFrequencyMap::FrequencyOf(const BasicBlock* form,
                                     const BasicBlock* to) const {
  auto const it = map_.find(std::make_pair(form, to));
  return it == map_.end() ? 0.0 : it->second;
}

//////////////////////////////////////////////////////////////////////
//
// StaticPredictor
//
StaticPredictor::StaticPredictor(ScheduleEditor* editor)
    : ScheduleEditor::User(editor) {
}

StaticPredictor::~StaticPredictor() {
}

void StaticPredictor::Predict(const BasicBlock* from, double frequency) {
  auto const last_node = from->last_node();
  switch (last_node->opcode()) {
    case Opcode::Exit:
      return;
    case Opcode::Ret:
    case Opcode::Throw:
    case Opcode::Unreachable:
      SetFrequency(from,
                   BlockOf(from->last_node()->use_edges().begin()->from()),
                   frequency);
      return;
    case Opcode::If: {
      auto const true_target = BlockOf(TrueTargetOf(from));
      auto const false_target = BlockOf(FalseTargetOf(from));
      auto const true_frequency = frequency * EstimateByCondition(last_node);
      SetFrequency(from, true_target, true_frequency);
      SetFrequency(from, false_target, frequency - true_frequency);
      return;
    }
    case Opcode::Jump: {
      SetFrequency(from, BlockOf(JumpTargetOf(from)), frequency);
      return;
    }
    case Opcode::Switch:
      NOTREACHED() << "NYI " << *last_node;
      return;
  }
  NOTREACHED() << *last_node;
}

std::unique_ptr<EdgeFrequencyMap> StaticPredictor::Run() {
  auto const blocks =
      ControlFlowGraph::Sorter::SortByReversePostOrder(control_flow_graph());
  for (auto const block : blocks) {
    auto frequency = LoopDepthOf(block) * 1000.0;
    for (auto const use_edge : block->first_node()->use_edges()) {
      auto const predecessor = BlockOf(use_edge->from());
      frequency += edge_map_.FrequencyOf(predecessor, block);
    }
    Predict(block, frequency);
  }
  return edge_map_.Finish();
}

void StaticPredictor::SetFrequency(const BasicBlock* from,
                                   const BasicBlock* to,
                                   double frequency) {
  edge_map_.Add(from, to, frequency);
}

}  // namespace optimizer
}  // namespace elang
