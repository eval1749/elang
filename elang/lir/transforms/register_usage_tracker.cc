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
RegisterUsageTracker::RegisterUsageTracker(const Editor* editor)
    : post_dominator_tree_(editor->BuildPostDominatorTree()),
      use_def_list_(UseDefListBuilder(editor->function()).Build()) {
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
  auto const block = instr->basic_block();
  auto candidate = instr;
  for (auto const user : use_def_list_.UsersOf(input)) {
    if (user->index() < candidate->index())
      continue;
    if (user->basic_block() != block ||
        !post_dominator_tree_.Dominates(block, user->basic_block())) {
      continue;
    }
    candidate = user;
  }
  return candidate == instr ? nullptr : candidate;
}

}  // namespace lir
}  // namespace elang
