// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/analysis/liveness_analyzer.h"

#include "base/logging.h"
#include "elang/base/analysis/data_flow_solver.h"
#include "elang/base/analysis/liveness_builder.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

std::unique_ptr<LivenessCollection<BasicBlock*, Value>>
AnalyzeLiveness(Function* function) {
  LivenessBuilder<BasicBlock*, Value> builder;

  // List up registers
  for (auto const block : function->basic_blocks()) {
    for (auto const phi_instruction : block->phi_instructions())
      builder.AddVariable(phi_instruction->output(0));
    for (auto const instruction : block->instructions()) {
      for (auto output : instruction->outputs())
        builder.AddVariable(output);
    }
  }

  // Populate initial liveness for each block.
  for (auto const block : function->basic_blocks())
    builder.AddNode(block);

  // Analyze use and kill in |block|.
  for (auto const block : function->basic_blocks()) {
    auto const liveness = builder.Edit(block);
    for (auto const phi_instruction : block->phi_instructions())
      builder.MarkKill(liveness, phi_instruction->output(0));
    for (auto const instruction : block->instructions()) {
      for (auto const input : instruction->inputs())
        builder.MarkUse(liveness, input);
      for (auto output : instruction->outputs())
        builder.MarkKill(liveness, output);
    }

    // Mark phi input in successor block in In(successor).
    for (auto const successor : block->successors()) {
      auto const liveness = builder.Edit(successor);
      for (auto const phi_instruction : successor->phi_instructions()) {
        for (auto const input : phi_instruction->phi_inputs())
          builder.MarkUse(liveness, input->value());
      }
    }
  }

  auto collection = builder.Finish();
  DataFlowSolver<Function, Value>(function, collection.get()).SolveBackward();
  return std::move(collection);
}

}  // namespace lir
}  // namespace elang
