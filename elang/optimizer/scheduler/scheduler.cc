// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <unordered_set>
#include <vector>

#include "elang/optimizer/scheduler/scheduler.h"

#include "base/logging.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/formatters/graphviz_formatter.h"
#include "elang/optimizer/formatters/text_formatter.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/block_layouter.h"
#include "elang/optimizer/scheduler/cfg_builder.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/optimizer/scheduler/schedule_editor.h"
#include "elang/optimizer/scheduler/static_predictor.h"
#include "elang/optimizer/scheduler/visual_schedule.h"

namespace elang {
namespace optimizer {

namespace {

bool IsPinned(const Node* node) {
  if (node->IsControl() || node->IsEffect())
    return true;
  // Since all nodes have at least one input except for |EntryNode|, it is safe
  // to use |input(0)|.
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
  if (!block) {
    if (!node->is<TupleNode>())
      DVLOG(0) << "Should be replaced with literal: " << *node;
    return;
  }
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
      DCHECK(node->is<PhiNode>() || node->is<EffectPhiNode>()) << *node;
      return;
    }
    editor()->AppendNode(block, node);
    return;
  }

  // Find Least Common Ancestor of users of |node|.
  BasicBlock* lca_block = nullptr;
  for (auto const edge : node->use_edges()) {
    auto const user = edge->from();
    if (auto const phi = user->as<PhiNode>()) {
      // This loop could be removed (and the code made asymptotically faster)
      // by using complex data structures. In practice it is never a bottleneck.
      for (auto phi_operand : phi->phi_inputs()) {
        if (phi_operand->value() != node)
          continue;
        auto const from_block = BlockOf(phi_operand->control());
        lca_block =
            lca_block ? CommonAncestorOf(lca_block, from_block) : from_block;
      }
      continue;
    }
    auto const use_block = BlockOf(user);
    lca_block = lca_block ? CommonAncestorOf(lca_block, use_block) : use_block;
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
class NodePlacer final : public api::Pass, public ScheduleEditor::User {
 public:
  explicit NodePlacer(api::PassController* pass_controller,
                      ScheduleEditor* editor,
                      const std::vector<BasicBlock*>& blocks);
  ~NodePlacer() = default;

  void PrintTo(std::ostream* ostream) const;
  void Run();

 private:
  bool IsUsedInBlock(Node* node, BasicBlock* block) const;
  void PlaceNode(Node* node);
  void ScheduleInBlock(BasicBlock* block);

  // api::Pass
  base::StringPiece name() const final;

  const std::vector<BasicBlock*>& blocks_;
  std::vector<Node*> nodes_;
  std::unordered_set<Node*> placed_;

  DISALLOW_COPY_AND_ASSIGN(NodePlacer);
};

NodePlacer::NodePlacer(api::PassController* pass_controller,
                       ScheduleEditor* editor,
                       const std::vector<BasicBlock*>& blocks)
    : Pass(pass_controller), ScheduleEditor::User(editor), blocks_(blocks) {
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

void NodePlacer::PlaceNode(Node* node) {
  DCHECK(node->IsUsed()) << *node;
  if (placed_.count(node))
    return;
  // Place literal inputs before |node|.
  for (auto const input : node->inputs()) {
    if (input->IsLiteral() || BlockOf(input))
      continue;
    DCHECK(input->SelectUserIfOne() == node) << *input;
    placed_.insert(input);
    nodes_.push_back(input);
  }
  placed_.insert(node);
  nodes_.push_back(node);
}

void NodePlacer::PrintTo(std::ostream* ostream) const {
  *ostream << "Extra nodes:" << std::endl;
  for (auto const node : nodes_) {
    if (placed_.count(node))
      continue;
    *ostream << "  " << *node << std::endl;
  }
}

std::ostream& operator<<(std::ostream& ostream, const NodePlacer& pass) {
  pass.PrintTo(&ostream);
  return ostream;
}

void NodePlacer::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return;
  for (auto const block : blocks_)
    ScheduleInBlock(block);
  DCHECK_EQ(placed_.size(), nodes_.size()) << *this;
  editor()->DidPlaceNodes(nodes_);
}

void NodePlacer::ScheduleInBlock(BasicBlock* block) {
  WorkList<Node> pending_list;
  auto end_node = static_cast<Node*>(nullptr);
  auto first = true;
  for (auto const node : block->nodes()) {
    if (!first) {
      if (node->IsBlockEnd()) {
        DCHECK(!end_node) << *node << " " << *end_node;
        end_node = node;
        continue;
      }
      pending_list.Push(node);
      continue;
    }
    first = false;
    DCHECK(node->IsBlockStart()) << *node;
    PlaceNode(node);
    if (auto const phi_owner = node->as<PhiOwnerNode>()) {
      if (auto const effect_phi = phi_owner->effect_phi()) {
        if (effect_phi->IsUsed()) {
          PlaceNode(effect_phi);
        }
      }
      for (auto const phi : phi_owner->phi_nodes()) {
        if (!phi->IsUsed())
          continue;
        PlaceNode(phi);
      }
    }
  }
  DCHECK(end_node) << block;

  while (!pending_list.empty()) {
    WorkList<Node> work_list;
    while (!pending_list.empty())
      work_list.Push(pending_list.Pop());
    while (!work_list.empty()) {
      auto const node = work_list.Pop();
      for (auto const input : node->inputs()) {
        if (input->IsLiteral() || BlockOf(input) != block ||
            placed_.count(input)) {
          continue;
        }
        pending_list.Push(node);
        break;
      }
      if (pending_list.Contains(node))
        continue;
      PlaceNode(node);
    }
  }

  DCHECK(!placed_.count(end_node)) << *end_node;
  PlaceNode(end_node);
}

// api::Pass
base::StringPiece NodePlacer::name() const {
  return "node_placement";
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Scheduler
//
Scheduler::Scheduler(api::PassController* pass_controller, Schedule* schedule)
    : Pass(pass_controller), editor_(new ScheduleEditor(schedule)) {
  DCHECK(schedule);
}

Scheduler::~Scheduler() {
}

// The entry point
void Scheduler::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return;
  CfgBuilder(editor_.get()).Run();
  EarlyScheduler(editor_.get()).Run();
  LateScheduler(editor_.get()).Run();
  auto const edge_map = StaticPredictor(pass_controller(), editor_.get()).Run();
  auto const blocks =
      BlockLayouter(pass_controller(), editor_.get(), edge_map.get()).Run();
  NodePlacer(pass_controller(), editor_.get(), blocks).Run();
}

// api::Pass
base::StringPiece Scheduler::name() const {
  return "schedule";
}

void Scheduler::DumpBeforePass(const api::PassDumpContext& context) {
  auto& ostream = *context.ostream;
  if (context.IsGraph()) {
    ostream << AsGraphviz(editor_->function());
    return;
  }
  ostream << AsReversePostOrder(editor_->function());
}

void Scheduler::DumpAfterPass(const api::PassDumpContext& context) {
  auto& ostream = *context.ostream;
  if (context.IsGraph()) {
    ostream << AsVisual(editor_->schedule());
    return;
  }
  ostream << editor_->schedule();
}

}  // namespace optimizer
}  // namespace elang
