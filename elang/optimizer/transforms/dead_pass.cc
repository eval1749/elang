// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/optimizer/transforms/dead_pass.h"

#include "elang/base/work_list.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/formatters/graphviz_formatter.h"
#include "elang/optimizer/formatters/text_formatter.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

namespace {

//////////////////////////////////////////////////////////////////////
//
// DeadNodeCollector
//
class DeadNodeCollector final : public NodeVisitor {
 public:
  DeadNodeCollector(const Function* function,
                    const std::vector<bool>& lives,
                    WorkList<Node>* dead_nodes);
  ~DeadNodeCollector() final = default;

  void Run();

 private:
  // NodeVisitor
  void DoDefaultVisit(Node* node) final;

  WorkList<Node>& dead_nodes_;
  const Function* const function_;
  const std::vector<bool>& lives_;

  DISALLOW_COPY_AND_ASSIGN(DeadNodeCollector);
};

DeadNodeCollector::DeadNodeCollector(const Function* function,
                                     const std::vector<bool>& lives,
                                     WorkList<Node>* dead_nodes)
    : dead_nodes_(*dead_nodes), function_(function), lives_(lives) {
}

void DeadNodeCollector::Run() {
  DepthFirstTraversal<OnUseEdge, const Function> walker;
  walker.Traverse(function_, this);
}

// NodeVisitor
void DeadNodeCollector::DoDefaultVisit(Node* node) {
  if (lives_[node->id()])
    return;
  dead_nodes_.Push(node);
}

//////////////////////////////////////////////////////////////////////
//
// LiveNodeCollector
//
class LiveNodeCollector final : public NodeVisitor {
 public:
  LiveNodeCollector(const Function* function, std::vector<bool>* lives);
  ~LiveNodeCollector() final = default;

  void Run();

 private:
  // NodeVisitor
  void DoDefaultVisit(Node* node) final;

  const Function* const function_;
  std::vector<bool>& lives_;

  DISALLOW_COPY_AND_ASSIGN(LiveNodeCollector);
};

LiveNodeCollector::LiveNodeCollector(const Function* function,
                                     std::vector<bool>* lives)
    : function_(function), lives_(*lives) {
}

void LiveNodeCollector::Run() {
  DepthFirstTraversal<OnInputEdge, const Function> walker;
  walker.Traverse(function_, this);
}

// NodeVisitor
void LiveNodeCollector::DoDefaultVisit(Node* node) {
  lives_[node->id()] = true;
}

}  // namespace

DeadPass::DeadPass(api::PassObserver* observer, Editor* editor)
    : api::Pass(observer), editor_(*editor) {
}

DeadPass::~DeadPass() {
}

void DeadPass::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return;
  auto const function = editor_.function();

  std::vector<bool> lives(function->max_node_id() + 1);
  LiveNodeCollector(function, &lives).Run();

  WorkList<Node> dead_nodes;
  DeadNodeCollector(function, lives, &dead_nodes).Run();

  while (!dead_nodes.empty()) {
    auto const dead_node = dead_nodes.Pop();
    DVLOG(0) << "Dead " << *dead_node;
    editor_.Discard(dead_node);
  }
}

// api::Pass
base::StringPiece DeadPass::name() const {
  return "dead";
}

void DeadPass::DumpBeforePass(const api::PassDumpContext& context) {
  auto& ostream = *context.ostream;
  if (context.IsGraph()) {
    ostream << AsGraphviz(editor_.function());
    return;
  }
  ostream << AsReversePostOrder(editor_.function());
}

void DeadPass::DumpAfterPass(const api::PassDumpContext& context) {
  auto& ostream = *context.ostream;
  if (context.IsGraph()) {
    ostream << AsGraphviz(editor_.function());
    return;
  }
  ostream << AsReversePostOrder(editor_.function());
}

}  // namespace optimizer
}  // namespace elang
