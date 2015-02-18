// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/prepare_phi_inversion_pass.h"

#include "base/logging.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// PreparePhiInversionPass
//
PreparePhiInversionPass::PreparePhiInversionPass(Editor* editor)
    : FunctionPass(editor) {
}

PreparePhiInversionPass::~PreparePhiInversionPass() {
}

base::StringPiece PreparePhiInversionPass::name() const {
  return "prepare_phi_inversion";
}

// Inserts a new block between predecessors of phi block if predecessor block
// has more than one successors.
void PreparePhiInversionPass::RunOnFunction() {
  // Since we modify predecessors of phi block, we can't use |predecessors()|
  // during inserting new block.
  struct WorkItem {
    BasicBlock* phi_block;
    BasicBlock* predecessor;
  };
  std::vector<WorkItem> items;

  // Collection phase
  for (auto const block : function()->basic_blocks()) {
    if (block->phi_instructions().empty())
      continue;
    for (auto const predecessor : block->predecessors()) {
      if (!predecessor->HasMoreThanOneSuccessor())
        continue;
      items.push_back({block, predecessor});
    }
  }

  // Rewriting phase
  for (auto const item : items) {
    // Insert new block after |predecessor|.
    auto const new_block = editor()->NewBasicBlock(item.predecessor->next());

    // |new_block| to |phi_block|
    editor()->Edit(new_block);
    editor()->SetJump(item.phi_block);
    editor()->Commit();

    // Redirect edge |predecessor| => |phi_block| to |new_block| => |phi_block|
    editor()->Edit(item.predecessor);
    auto const last = item.predecessor->last_instruction();
    auto position = 0;
    for (auto const target : last->block_operands()) {
      if (target == item.phi_block)
        editor()->SetBlockOperand(last, position, new_block);
      ++position;
    }
    editor()->Commit();

    // Update phi inputs
    editor()->Edit(item.phi_block);
    editor()->ReplacePhiInputs(new_block, item.predecessor);
    editor()->Commit();
  }
}

}  // namespace lir
}  // namespace elang
