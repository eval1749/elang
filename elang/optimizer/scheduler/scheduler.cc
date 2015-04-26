// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/optimizer/scheduler/scheduler.h"

#include "base/logging.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/cfg_builder.h"
#include "elang/optimizer/scheduler/schedule_editor.h"

namespace elang {
namespace optimizer {

namespace {

bool IsPinned(const Node* node) {
  if (node->IsControl() || node->IsEffect())
    return true;
  // All nodes have at least one input except for |EntryNode|.
  if (node->input(0)->IsControl())
    return true;
  return node->opcode() == Opcode::Phi || node->opcode() == Opcode::EffectPhi;
}

//////////////////////////////////////////////////////////////////////
//
// EarlyScheduler
//
class EarlyScheduler final : public NodeVisitor, public ScheduleEditor::User {
 public:
  explicit EarlyScheduler(ScheduleEditor* editor);
  ~EarlyScheduler() final = default;

  void Run();

 private:
  void DoDefaultVisit(Node* node) final;

  DISALLOW_COPY_AND_ASSIGN(EarlyScheduler);
};

EarlyScheduler::EarlyScheduler(ScheduleEditor* editor)
    : ScheduleEditor::User(editor) {
}

void EarlyScheduler::Run() {
  DepthFirstTraversal<OnInputEdge, const Function> walker;
  walker.Traverse(function(), this);
}

// NodeVisitor implementation
void EarlyScheduler::DoDefaultVisit(Node* node) {
  if (node->IsLiteral() || BlockOf(node))
    return;
  // Place |node| into deepest block in dominator tree.
  BasicBlock* block = nullptr;
  for (auto const input : node->inputs()) {
    if (input->IsLiteral())
      continue;
    auto input_block = BlockOf(input);
    if (!block) {
      block = input_block;
      continue;
    }
    if (DepthOf(block) < DepthOf(input_block))
      block = input_block;
  }
  DCHECK(block) << *node;
  editor()->SetBlockOf(node, block);
}

//////////////////////////////////////////////////////////////////////
//
// LateScheduler
//
class LateScheduler final : public NodeVisitor, public ScheduleEditor::User {
 public:
  explicit LateScheduler(ScheduleEditor* editor);
  ~LateScheduler() final = default;

  void Run();

 private:
  void DoDefaultVisit(Node* node) final;

  DISALLOW_COPY_AND_ASSIGN(LateScheduler);
};

LateScheduler::LateScheduler(ScheduleEditor* editor)
    : ScheduleEditor::User(editor) {
}

void LateScheduler::Run() {
  DepthFirstTraversal<OnUseEdge, const Function> walker;
  walker.Traverse(function(), this);
}

// NodeVisitor implementation
void LateScheduler::DoDefaultVisit(Node* node) {
  if (IsPinned(node) || node->IsLiteral())
    return;
  // Find Least Common Ancestor of users of |node|.
  BasicBlock* lca_block = nullptr;
  for (auto const edge : node->use_edges()) {
    if (auto const phi = edge->from()->as<PhiNode>()) {
      // This loop could be removed (and the code made asymptotically faster)
      // by using complex data structures. In practice it is never a bottleneck.
      for (auto phi_operand : phi->phi_inputs()) {
        if (phi_operand->value() != node)
          continue;
        lca_block =
            CommonAncestorOf(lca_block, BlockOf(phi_operand->control()));
      }
      continue;
    }
    auto const use_block = BlockOf(edge->to());
    if (!lca_block) {
      lca_block = use_block;
      continue;
    }
    lca_block = CommonAncestorOf(lca_block, use_block);
  }
  DCHECK(lca_block) << *node;
  auto best_block = lca_block;
  auto const block = BlockOf(node);
  for (auto runner = lca_block; runner != block; runner = DominatorOf(runner)) {
    if (LoopDepthOf(best_block) <= LoopDepthOf(lca_block))
      continue;
    best_block = lca_block;
  }
  // TODO(eval1749) Split |node| if it is partially dead.
  editor()->SetBlockOf(node, best_block);
}

//////////////////////////////////////////////////////////////////////
//
// NodePlacer
//
class NodePlacer final : public NodeVisitor, public ScheduleEditor::User {
 public:
  explicit NodePlacer(ScheduleEditor* editor);
  ~NodePlacer() final = default;

  void Run();

 private:
  void DoDefaultVisit(Node* node) final;

  std::vector<Node*> nodes_;

  DISALLOW_COPY_AND_ASSIGN(NodePlacer);
};

NodePlacer::NodePlacer(ScheduleEditor* editor) : ScheduleEditor::User(editor) {
  nodes_.reserve(function()->max_node_id());
}

void NodePlacer::Run() {
  DepthFirstTraversal<OnInputEdge, const Function> walker;
  walker.Traverse(function(), this);
  for (auto const node : nodes_) {
    if (node->IsLiteral())
      continue;
    editor()->AppendNode(BlockOf(node), node);
  }
  editor()->DidPlaceNodes();
}

// NodeVisitor implementation
void NodePlacer::DoDefaultVisit(Node* node) {
  nodes_.push_back(node);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Scheduler
//
Scheduler::Scheduler(Schedule* schedule) : schedule_(*schedule) {
  DCHECK(schedule);
}

Scheduler::~Scheduler() {
}

// The entry point
void Scheduler::Run() {
  ScheduleEditor editor(&schedule_);
  CfgBuilder(&editor).Run();
  EarlyScheduler(&editor).Run();
  LateScheduler(&editor).Run();
  NodePlacer(&editor).Run();
}

}  // namespace optimizer
}  // namespace elang
