// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/analysis/liveness_analyzer.h"

#include "base/logging.h"
#include "elang/base/liveness.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LivenessAnalyzer
//
LivenessAnalyzer::LivenessAnalyzer(const LiteralMap* literals)
    : literals_(literals) {
}

LivenessAnalyzer::~LivenessAnalyzer() {
}

std::unique_ptr<LivenessAnalyzer::Collection> LivenessAnalyzer::Analyze(
    Function* function) {
  for (auto const block : function->basic_blocks()) {
    auto const liveness = builder_.LivenessOf(block);
    for (auto const phi_instruction : block->phi_instructions())
      builder_.MarkKill(liveness, phi_instruction->output(0));
    for (auto const instruction : block->instructions()) {
      for (auto const input : instruction->inputs())
        builder_.MarkUse(liveness, input);
      for (auto output : instruction->outputs())
        builder_.MarkKill(liveness, output);
    }
    for (auto const successor : block->successors()) {
      auto const liveness = builder_.LivenessOf(successor);
      for (auto const phi_instruction : successor->phi_instructions()) {
        for (auto const input : phi_instruction->phi_inputs())
          builder_.MarkUse(liveness, input->value());
      }
    }
  }
  return std::move(builder_.Finish());
}

void LivenessAnalyzer::Initialize(Function* function) {
  for (auto const block : function->basic_blocks()) {
    for (auto const phi_instruction : block->phi_instructions())
      builder_.AddValue(phi_instruction->output(0));
    for (auto const instruction : block->instructions()) {
      for (auto output : instruction->outputs())
        builder_.AddValue(output);
    }
  }

  for (auto const block : function->basic_blocks())
    builder_.AddBlock(block);
}

}  // namespace lir
}  // namespace elang
