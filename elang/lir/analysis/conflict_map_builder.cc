// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/analysis/conflict_map_builder.h"

#include "base/containers/adapters.h"
#include "base/logging.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/base/bit_set.h"
#include "elang/lir/analysis/conflict_map.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
Value Normalize(Value value) {
  if (value.is_physical())
    return Target::NaturalRegisterOf(value);
  return value;
}
}  // namespace

ConflictMapBuilder::ConflictMapBuilder(const Editor* editor)
    : editor_(editor), liveness_map_(editor->AnalyzeLiveness()) {
}

ConflictMapBuilder::~ConflictMapBuilder() {
}

// Build conflict map by reverse instruction list scanning.
ConflictMap ConflictMapBuilder::Build() {
  ConflictMap conflict_map;
  for (auto const variable : liveness_map_.variables())
    conflict_map.sets_.MakeSet(variable);

  auto const live_registers = liveness_map_.work();
  for (auto const block : editor_->function()->basic_blocks()) {
    auto& liveness = liveness_map_.LivenessOf(block);

    // Members in Live-Out conflict to other members.
    UpdateConflictMapFromLiveness(&conflict_map, liveness.out());

    for (auto const instr : base::Reversed(block->instructions())) {
      for (auto const output : instr->outputs())
        live_registers->Remove(liveness_map_.NumberOf(Normalize(output)));

      for (auto const input : instr->inputs()) {
        if (!input.is_register())
          continue;
        for (auto const number : *live_registers) {
          auto const live = liveness_map_.VariableOf(number);
          conflict_map.sets_.Union(Normalize(input), live);
        }
        auto const number = liveness_map_.NumberOf(Normalize(input));
        DCHECK_GE(number, 0) << input << " in " << *instr
                             << " doesn't have liveness!";
        live_registers->Add(number);
      }
    }

    // Phi outputs conflict with Live-In and other phi outputs.
    UpdateConflictMapFromLiveness(&conflict_map, liveness.in());

    // Phi outputs conflict with Live-In and other phi outputs.
    for (auto const phi : block->phi_instructions()) {
      auto const output = phi->output(0);
      DCHECK(!output.is_physical()) << *phi;
      for (auto const number : *live_registers) {
        auto const live = liveness_map_.VariableOf(number);
        conflict_map.sets_.Union(output, live);
      }
    }
  }
  return std::move(conflict_map);
}

void ConflictMapBuilder::UpdateConflictMapFromLiveness(
    ConflictMap* conflict_map,
    const BitSet& lives) {
  auto const live_registers = liveness_map_.work();
  live_registers->Clear();
  Value first;
  for (auto const number : lives) {
    auto const live = liveness_map_.VariableOf(number);
    if (first.is_void())
      first = live;
    else
      conflict_map->sets_.Union(first, live);
    live_registers->Add(number);
  }
}

}  // namespace lir
}  // namespace elang
