// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_usage_tracker.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree.h"
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
    : dominator_tree_(editor->BuildDominatorTree()),
      post_dominator_tree_(editor->BuildPostDominatorTree()),
      use_def_list_(UseDefListBuilder(editor->function()).Build()) {
  editor->AssignIndex();
}

RegisterUsageTracker::~RegisterUsageTracker() {
}

bool RegisterUsageTracker::IsUsedAfter(Value input, Instruction* instr) const {
  DCHECK(input.is_virtual());
  auto const block = instr->basic_block();
  for (auto const user : use_def_list_.UsersOf(input)) {
    if (user->basic_block() == block) {
      if (user->index() <= instr->index())
        continue;
      return true;
    }
    if (!post_dominator_tree_.Dominates(block, user->basic_block()))
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
  auto candidate = instr;
  for (auto const user : use_def_list_.UsersOf(input)) {
    DCHECK(user->index()) << *user;
    if (user->basic_block() == block) {
      if (candidate->index() >= user->index())
        continue;
      candidate = user;
      continue;
    }
    if (dominator_tree_.Dominates(block, user->basic_block())) {
      candidate = user;
      continue;
    }
    if (post_dominator_tree_.Dominates(user->basic_block(), block)) {
      candidate = user;
      continue;
    }
  }
  return candidate == instr ? nullptr : candidate;
}

}  // namespace lir
}  // namespace elang
