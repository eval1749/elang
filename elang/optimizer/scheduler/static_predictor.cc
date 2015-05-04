// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>
#include <vector>

#include "elang/optimizer/scheduler/static_predictor.h"

#include "elang/base/graphs/graph_sorter.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/edge_profile_editor.h"

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
  return last_node->SelectUserIfOne();
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
// StaticPredictor
//
StaticPredictor::StaticPredictor(api::PassObserver* observer,
                                 ScheduleEditor* editor)
    : Pass(observer),
      ScheduleEditor::User(editor),
      edge_profile_(new EdgeProfileEditor()) {
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
      // Set frequency for pseudo edge to exit block.
      SetFrequency(from, BlockOf(last_node->SelectUserIfOne()), frequency);
      return;
    case Opcode::If: {
      auto const true_block = BlockOf(TrueTargetOf(from));
      auto const false_block = BlockOf(FalseTargetOf(from));
      if (LoopDepthOf(true_block) != LoopDepthOf(false_block)) {
        SetBranchFrequency(
            from, true_block, false_block, frequency,
            LoopDepthOf(true_block) < LoopDepthOf(false_block) ? 0.001 : 0.999);
        return;
      }

      auto const probability = EstimateByCondition(last_node);
      if (probability != 0.5) {
        SetBranchFrequency(from, true_block, false_block, frequency,
                           probability);
        return;
      }
      SetBranchFrequency(
          from, true_block, false_block, frequency,
          PostDepthOf(true_block) < PostDepthOf(false_block) ? 0.1 : 0.9);
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

std::unique_ptr<EdgeProfile> StaticPredictor::Run() {
  {
    RunScope scope(this);
    auto const blocks =
        ControlFlowGraph::Sorter::SortByReversePostOrder(control_flow_graph());
    for (auto const block : blocks) {
      auto total_frequency = 0.0;
      for (auto const control : block->first_node()->inputs()) {
        DCHECK(control->IsControl());
        auto const predecessor = BlockOf(control);
        auto const frequency =
            edge_profile_->Has(predecessor, block)
                ? edge_profile_->FrequencyOf(predecessor, block)
                : LoopDepthOf(block) * 1000;
        total_frequency += frequency;
      }
      if (block->first_node()->opcode() == Opcode::Entry)
        total_frequency = 1.0;
      Predict(block, total_frequency);
    }
  }
  return edge_profile_->Finish();
}

void StaticPredictor::SetFrequency(const BasicBlock* from,
                                   const BasicBlock* to,
                                   double frequency) {
  edge_profile_->Add(from, to, frequency);
}

void StaticPredictor::SetBranchFrequency(const BasicBlock* block,
                                         const BasicBlock* true_block,
                                         const BasicBlock* false_block,
                                         double frequency,
                                         double probability) {
  auto const true_frequency = frequency * probability;
  SetFrequency(block, true_block, true_frequency);
  SetFrequency(block, false_block, frequency - true_frequency);
}

// api::Pass
base::StringPiece StaticPredictor::name() const {
  return "static_predictor";
}

void StaticPredictor::DumpAfterPass(const api::PassDumpContext& context) {
  auto& ostream = *context.ostream;
  using Edge = EdgeProfile::Edge;
  std::vector<Edge> edges;
  edges.reserve(edge_profile_->all_edges().size());
  for (auto const data : edge_profile_->all_edges())
    edges.push_back(data.first);
  std::sort(edges.begin(), edges.end(),
            [](const Edge& edge1, const Edge& edge2) {
              if (edge1.first->id() == edge2.first->id())
                return edge1.second->id() < edge2.second->id();
              return edge1.first->id() < edge2.first->id();
            });
  ostream << "Static prediction" << std::endl;
  for (auto const& edge : edges) {
    auto const from = edge.first;
    auto const to = edge.second;
    ostream << "  " << from << "/" << LoopDepthOf(from) << "/"
            << PostDepthOf(from) << " -> " << to << "/" << LoopDepthOf(to)
            << "/" << PostDepthOf(to) << " "
            << edge_profile_->FrequencyOf(edge.first, edge.second) << std::endl;
  }
}

}  // namespace optimizer
}  // namespace elang
