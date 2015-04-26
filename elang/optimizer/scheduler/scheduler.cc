// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>
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
    DCHECK(input_block) << *input << " in " << *node;
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
  if (node->IsLiteral())
    return;
  if (IsPinned(node)) {
    auto const block = BlockOf(node);
    if (!block) {
      DCHECK(node->is<PhiNode>() || node->is<EffectPhiNode>());
      return;
    }
    editor()->AppendNode(block, node);
    return;
  }
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
    if (LoopDepthOf(best_block) <= LoopDepthOf(runner))
      continue;
    best_block = runner;
  }
  // TODO(eval1749) Split |node| if it is partially dead.
  editor()->SetBlockOf(node, best_block);
  editor()->AppendNode(best_block, node);
}

//////////////////////////////////////////////////////////////////////
//
// NodePlacer
//
class NodePlacer final : public ScheduleEditor::User {
 public:
  explicit NodePlacer(ScheduleEditor* editor);
  ~NodePlacer() = default;

  void Run();

 private:
  bool IsUsedInBlock(Node* node, BasicBlock* block) const;
  void ScheduleInBlock(BasicBlock* block);

  std::vector<Node*> nodes_;

  DISALLOW_COPY_AND_ASSIGN(NodePlacer);
};

NodePlacer::NodePlacer(ScheduleEditor* editor) : ScheduleEditor::User(editor) {
  nodes_.reserve(function()->max_node_id());
}

bool NodePlacer::IsUsedInBlock(Node* node, BasicBlock* block) const {
  for (auto const edge : node->use_edges()) {
    auto const user = edge->from();
    if (BlockOf(user) == block)
      return true;
  }
  return false;
}

void NodePlacer::Run() {
  for (auto const block : editor()->blocks())
    ScheduleInBlock(block);
  editor()->DidPlaceNodes(nodes_);
}

void NodePlacer::ScheduleInBlock(BasicBlock* block) {
  std::unordered_set<Node*> placed;
  WorkList<Node> pending_list;
  auto end_node = static_cast<Node*>(nullptr);
  auto first = true;
  for (auto const node : block->nodes()) {
    if (!first) {
      if (node->IsBlockEnd()) {
        DCHECK(!end_node);
        end_node = node;
        continue;
      }
      pending_list.Push(node);
      continue;
    }
    first = false;
    DCHECK(node->IsBlockStart());
    nodes_.push_back(node);
    placed.insert(node);
    if (auto const phi_owner = node->as<PhiOwnerNode>()) {
      if (auto const effect_phi = phi_owner->effect_phi()) {
        nodes_.push_back(effect_phi);
        placed.insert(effect_phi);
      }
      for (auto const phi : phi_owner->phi_nodes()) {
        nodes_.push_back(phi);
        placed.insert(phi);
      }
    }
  }
  DCHECK(end_node);

  while (!pending_list.empty()) {
    WorkList<Node> work_list;
    while (!pending_list.empty())
      work_list.Push(pending_list.Pop());
    while (!work_list.empty()) {
      auto const node = work_list.Pop();
      for (auto const input : node->inputs()) {
        if (input->IsLiteral() || BlockOf(input) != block ||
            placed.count(input)) {
          continue;
        }
        pending_list.Push(node);
        break;
      }
      if (!pending_list.Contains(node)) {
        nodes_.push_back(node);
        placed.insert(node);
      }
    }
  }

  nodes_.push_back(end_node);
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
