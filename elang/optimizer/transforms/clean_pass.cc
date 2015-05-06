// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/optimizer/transforms/clean_pass.h"

#include "elang/api/pass_controller.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/formatters/graphviz_formatter.h"
#include "elang/optimizer/formatters/text_formatter.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"

namespace elang {
namespace optimizer {

namespace {

// Collects nodes ending block.
class NodeCollector final : public NodeVisitor {
 public:
  NodeCollector() {}
  ~NodeCollector() = default;

  const std::vector<Node*>& nodes() const { return nodes_; }

 private:
  void DoDefaultVisit(Node* node) final {
    if (!node->IsBlockEnd())
      return;
    nodes_.push_back(node);
  }

  std::vector<Node*> nodes_;

  DISALLOW_COPY_AND_ASSIGN(NodeCollector);
};

struct PostOrderControlFlow {
  static std::vector<Node*> AdjacentEdgesOf(const Function* function,
                                            Node* node) {
    std::vector<Node*> users;
    for (auto const edge : node->use_edges()) {
      auto const user = edge->from()->as<Control>();
      if (!user)
        continue;
      users.push_back(user);
    }
    return std::move(users);
  }

  static Node* EdgeTo(Node* node) { return node; }

  static bool ShouldVisit(const Function* function, Node* node) { return true; }

  static Node* StartNodeOf(const Function* function) {
    return function->entry_node();
  }
};

bool IsEmptyBlock(const Node* last_node) {
  DCHECK(last_node->IsBlockEnd()) << *last_node;
  auto const merge_node = last_node->input(0)->as<PhiOwnerNode>();
  if (!merge_node)
    return false;
  return !merge_node->effect_phi() && merge_node->phi_nodes().empty();
}

// Returns true if a block is empty and ends with |IfNode|.
bool CanHoistBranch(const Node* first_node) {
  DCHECK(first_node->IsBlockStart()) << *first_node;
  auto const last_node = first_node->SelectUserIfOne();
  if (!last_node || !last_node->IsBlockEnd())
    return false;
  return last_node->opcode() == Opcode::If && IsEmptyBlock(last_node);
}

}  // namespace

CleanPass::CleanPass(Editor* editor)
    : api::Pass(editor->pass_controller()), changed_(false), editor_(*editor) {
}

CleanPass::~CleanPass() {
}

void CleanPass::Clean() {
  DepthFirstTraversal<PostOrderControlFlow, const Function> walker;
  NodeCollector collector;
  walker.Traverse(editor_.function(), &collector);

  // Process successors before processing |Node|.
  for (auto const& node : collector.nodes()) {
    switch (node->opcode()) {
      case Opcode::If:
        CleanIf(node);
        break;
      case Opcode::Jump:
        CleanJump(node);
        break;
    }
  }
  DCHECK(editor_.Validate()) << editor_;
}

void CleanPass::CleanIf(Node* if_node) {
  auto const true_start = if_node->SelectUser(Opcode::IfTrue);
  auto const true_end = true_start->SelectUserIfOne();
  if (!true_end || !true_end->IsBlockEnd())
    return;
  auto const false_start = if_node->SelectUser(Opcode::IfFalse);
  auto const false_end = false_start->SelectUserIfOne()->as<Control>();
  if (!false_end || !false_end->IsBlockEnd() ||
      false_end->opcode() != true_end->opcode()) {
    return;
  }

  if (true_end->opcode() == Opcode::Jump) {
    // Fold a redundant branch
    auto const merge_node = true_end->SelectUserIfOne()->as<PhiOwnerNode>();
    if (!merge_node || merge_node != false_end->SelectUserIfOne())
      return;
    WillChangeControlFlow("Fold a branch", if_node);
    editor_.ChangeInput(true_end, 0, if_node->input(0));
    editor_.RemoveControlInput(merge_node, false_end);
    DidChangeControlFlow("Fold a branch", if_node);
    CleanJump(true_end);
    return;
  }

  if (true_end->opcode() != Opcode::Ret ||
      true_end->input(1) != false_end->input(1)) {
    return;
  }
  // TODO(eval1749) We should have |SelectUser| for conditional assignment.
  if (true_end->input(2) != false_end->input(2))
    return;
  // True and false blocks return same value.
  WillChangeControlFlow("Combine ret", if_node);
  editor_.ChangeInput(true_end, 0, if_node->input(0));
  DidChangeControlFlow("Combine ret", if_node);
}

void CleanPass::CleanJump(Node* jump_node) {
  auto const target = jump_node->SelectUserIfOne()->as<PhiOwnerNode>();
  if (!target)
    return;
  auto const control = jump_node->input(0);
  if (IsEmptyBlock(jump_node)) {
    // Remove an empty block
    DCHECK(control->is<PhiOwnerNode>()) << *control;
    WillChangeControlFlow("Remove an empty block", control);
    for (auto const predecessor : control->inputs()) {
      auto const it = std::find(target->inputs().begin(),
                                target->inputs().end(), predecessor);
      if (it != target->inputs().end())
        continue;
      editor_.AppendInput(target, predecessor);
    }
    DidChangeControlFlow("Remove an empty block", target);
  }
  if (target->CountInputs() != 1)
    return;

  // Combine blocks
  WillChangeControlFlow("Combine blocks", target);
  if (auto const effect_phi = target->effect_phi()) {
    editor_.ReplaceAllUses(effect_phi->input(0), effect_phi);
    editor_.Discard(effect_phi);
  }
  for (auto const phi : target->phi_nodes()) {
    editor_.ReplaceAllUses(phi->input(0), phi);
    editor_.Discard(phi);
  }
  editor_.ReplaceAllUses(control, target);
  editor_.Discard(target);
  editor_.Discard(jump_node);
  DidChangeControlFlow("Combine blocks", control);
}

void CleanPass::DidChangeControlFlow(base::StringPiece message,
                                     const Node* node) {
  DVLOG(1) << "After " << message << ": " << *node;
  changed_ = true;
}

void CleanPass::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return;
  do {
    pass_controller()->DidStartPass(this);
    changed_ = false;
    Clean();
    pass_controller()->DidEndPass(this);
  } while (changed_);
}

void CleanPass::WillChangeControlFlow(base::StringPiece message,
                                      const Node* node) {
  DVLOG(1) << "Before " << message << ": " << *node;
  changed_ = true;
}

// api::Pass
base::StringPiece CleanPass::name() const {
  return "clean";
}

void CleanPass::DumpBeforePass(const api::PassDumpContext& context) {
  auto& ostream = *context.ostream;
  if (context.IsGraph()) {
    ostream << AsGraphviz(editor_.function());
    return;
  }
  ostream << AsReversePostOrder(editor_.function());
}

void CleanPass::DumpAfterPass(const api::PassDumpContext& context) {
  auto& ostream = *context.ostream;
  if (context.IsGraph()) {
    ostream << AsGraphviz(editor_.function());
    return;
  }
  ostream << AsReversePostOrder(editor_.function());
}

}  // namespace optimizer
}  // namespace elang
