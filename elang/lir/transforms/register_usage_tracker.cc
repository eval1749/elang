// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_usage_tracker.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/lir/analysis/use_def_list_builder.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// RegisterUsageTracker
//
RegisterUsageTracker::RegisterUsageTracker(Editor* editor)
    : liveness_(editor->AnalyzeLiveness()),
      use_def_list_(UseDefListBuilder(editor->function()).Build()) {
  editor->AssignIndex();
}

RegisterUsageTracker::~RegisterUsageTracker() {
}

bool RegisterUsageTracker::IsUsedAfter(Value input, Instruction* instr) const {
  DCHECK(input.is_virtual());
  auto const block = instr->basic_block();
  auto& live_out = liveness_.LivenessOf(block).out();
  if (live_out.Contains(liveness_.NumberOf(input)))
    return true;
  for (auto const user : use_def_list_.UsersOf(input)) {
    if (user->basic_block() != block)
      continue;
    if (user->index() > instr->index())
      return true;
  }
  return false;
}

Instruction* RegisterUsageTracker::NextUseAfter(Value input,
                                                Instruction* instr) const {
  DCHECK(input.is_virtual());
  // Other than 'entry' instruction, instructions should have non-zero index.
  DCHECK(!instr->is<EntryInstruction>());
  DCHECK(instr->index()) << "Function is modified after assigning index."
                         << " You should call Editor::AssignIndex() again.";
  auto const block = instr->basic_block();
  for (auto const user : use_def_list_.UsersOf(input)) {
    DCHECK(user->index()) << *user;
    if (user->basic_block() != block)
      continue;
    if (user->index() > instr->index())
      return user;
  }
  return nullptr;
}

}  // namespace lir
}  // namespace elang
