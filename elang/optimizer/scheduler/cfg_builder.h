// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_CFG_BUILDER_H_
#define ELANG_OPTIMIZER_SCHEDULER_CFG_BUILDER_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/base/zone_user.h"
#include "elang/optimizer/nodes_forward.h"
#include "elang/optimizer/node_visitor.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class Function;
class Schedule;

//////////////////////////////////////////////////////////////////////
//
// CfgBuilder
//
class CfgBuilder : public NodeVisitor, public ZoneUser {
 public:
  explicit CfgBuilder(Schedule* schedule);
  ~CfgBuilder();

  void Run();

 private:
  void AddCfgEdge(BasicBlock* from, BasicBlock* to);
  BasicBlock* BlockOf(Node* node);
  void EndBlock(Node* node);
  BasicBlock* NewBasicBlock(Node* start);
  void StartBlock(Node* node);

  // NodeVisitor protocol
  void DoDefaultVisit(Node* node) final;

  Node* block_end_node_;
  Schedule& schedule_;

  DISALLOW_COPY_AND_ASSIGN(CfgBuilder);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_CFG_BUILDER_H_
