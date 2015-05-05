// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_BLOCK_LAYOUTER_H_
#define ELANG_OPTIMIZER_SCHEDULER_BLOCK_LAYOUTER_H_

#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "elang/api/pass.h"
#include "elang/base/zone_owner.h"
#include "elang/optimizer/scheduler/schedule_editor.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class EdgeProfile;

//////////////////////////////////////////////////////////////////////
//
// BlockLayouter
//
// Implements block placement algorithm based on:
//  Engineering a Compiler, second edition
//  8.6.2 Global Code Placement
//  Keith D. Cooper, Linda Torczon
//  February 2011
//
class BlockLayouter final : public api::Pass,
                            public ScheduleEditor::User,
                            public ZoneOwner {
 public:
  // |Chain| is marked |public| for debugging purpose.
  class Chain;

  BlockLayouter(api::PassController* pass_controller,
                ScheduleEditor* editor,
                const EdgeProfile* edge_map);
  ~BlockLayouter();

  std::vector<BasicBlock*> Run();

 private:
  void BuildChain();
  Chain* ChainOf(const BasicBlock* block) const;
  std::vector<BasicBlock*> Layout();

  // api::Pass
  base::StringPiece name() const final;

  std::unordered_map<const BasicBlock*, Chain*> chain_map_;
  const EdgeProfile* const edge_map_;

  DISALLOW_COPY_AND_ASSIGN(BlockLayouter);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_BLOCK_LAYOUTER_H_
