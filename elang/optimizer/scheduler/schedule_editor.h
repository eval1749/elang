// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_EDITOR_H_
#define ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_EDITOR_H_

#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "elang/base/graphs/graph_editor.h"
#include "elang/base/zone_user.h"

namespace elang {

template <typename Graph>
class DominatorTree;

template <typename Graph>
class LoopTree;

namespace optimizer {

class BasicBlock;
class ControlFlowGraph;
class Function;
class Node;
class Schedule;

//////////////////////////////////////////////////////////////////////
//
// ScheduleEditor
//
class ScheduleEditor final : public ZoneUser {
 public:
  class User {
   protected:
    explicit User(ScheduleEditor* editor);
    ~User();

    const ScheduleEditor* editor() const { return &editor_; }
    ScheduleEditor* editor() { return &editor_; }

    ControlFlowGraph* control_flow_graph() const {
      return editor_.control_flow_graph();
    }
    Function* function() const { return editor_.function(); }

    BasicBlock* BlockOf(Node* node) const;
    BasicBlock* CommonAncestorOf(const BasicBlock* block1,
                                 const BasicBlock* block2) const;
    int DepthOf(const BasicBlock* block) const;
    BasicBlock* DominatorOf(const BasicBlock* block) const;
    int LoopDepthOf(const BasicBlock* block) const;
    BasicBlock* LoopHeaderOf(const BasicBlock* block) const;
    int PostDepthOf(const BasicBlock* block) const;

   private:
    ScheduleEditor& editor_;

    DISALLOW_COPY_AND_ASSIGN(User);
  };

  explicit ScheduleEditor(Schedule* scheduler);
  ~ScheduleEditor();

  ControlFlowGraph* control_flow_graph() const { return control_flow_graph_; }
  Function* function() const;
  const Schedule& schedule() const { return schedule_; }

  // Append |node| to |block|.
  void AppendNode(BasicBlock* block, Node* node);

  // Returns |BasicBlock| which |node| belongs to or |nullptr| if |node| isn't
  // yet associated to |BasicBlock|.
  BasicBlock* BlockOf(Node* node) const;

  // Returns common ancestor of |block1| and |block2| in dominator tree.
  BasicBlock* CommonAncestorOf(const BasicBlock* block1,
                               const BasicBlock* block2) const;

  // Returns depth of |block| in dominator tree.
  int DepthOf(const BasicBlock* block) const;

  // Tells no more modification of control flow graph.
  void DidBuildControlFlowGraph();

  // Tells all nodes are placed into blocks.
  void DidPlaceNodes(const std::vector<Node*>& nodes);

  // Returns immediate dominator of |block|.
  BasicBlock* DominatorOf(const BasicBlock* block) const;

  // Returns depth of |block| in loop nest tree.
  int LoopDepthOf(const BasicBlock* block) const;
  BasicBlock* LoopHeaderOf(const BasicBlock* block) const;

  // Returns |BasicBlock| associated to |start_node|
  BasicBlock* MapToBlock(Node* start_node);

  // Returns depth of |block| in post-dominator tree.
  int PostDepthOf(const BasicBlock* block) const;

  // Associates |node| to |block|.
  void SetBlockOf(Node* node, BasicBlock* block);

 private:
  using DominatorTree = DominatorTree<ControlFlowGraph>;
  using LoopTree = LoopTree<ControlFlowGraph>;

  // Mapping from |Node| node to |BasicBlock|
  std::unordered_map<Node*, BasicBlock*> block_map_;
  ControlFlowGraph* const control_flow_graph_;
  std::unique_ptr<DominatorTree> dominator_tree_;
  std::unique_ptr<LoopTree> loop_tree_;
  std::unique_ptr<DominatorTree> post_dominator_tree_;
  Schedule& schedule_;

  DISALLOW_COPY_AND_ASSIGN(ScheduleEditor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_EDITOR_H_
