// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_CFG_BUILDER_H_
#define ELANG_OPTIMIZER_SCHEDULER_CFG_BUILDER_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/base/graphs/graph_editor.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class Function;
class ScheduleEditor;

//////////////////////////////////////////////////////////////////////
//
// CfgBuilder
//
class CfgBuilder : public NodeVisitor {
 public:
  explicit CfgBuilder(ScheduleEditor* editor);
  ~CfgBuilder();

  void Run();

 private:
  BasicBlock* BlockOf(Node* node);
  void EndBlock(Node* node);
  void StartBlock(Node* node);

  // NodeVisitor protocol
  void DoDefaultVisit(Node* node) final;

  BasicBlock* block_;
  ControlFlowGraph::Editor cfg_editor_;
  ScheduleEditor& editor_;

  DISALLOW_COPY_AND_ASSIGN(CfgBuilder);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_CFG_BUILDER_H_
