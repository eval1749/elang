// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/prepare_phi_inversion.h"

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
  std::vector<BasicBlock*> blocks;
  for (auto const block : function()->basic_blocks()) {
    if (block->phi_instructions().empty())
      continue;
    for (auto const predecessor : block->predecessors()) {
      if (predecessor->HasMoreThanOneSuccessors()) {
        blocks.push_back(block);
        break;
      }
    }
  }

  for (auto const phi_block : blocks) {
    for (auto const predecessor : phi_block->predecessors()) {
      if (!predecessor->HasMoreThanOneSuccessors())
        continue;
      // Insert new block after |predecessor|.
      auto const new_block = editor()->NewBasicBlock(predecessor->next());
      editor()->Edit(new_block);
      editor()->SetJump(phi_block);
      editor()->Commit();

      editor()->Edit(predecessor);
      auto const last = predecessor->last_instruction();
      auto position = 0;
      for (auto const target : last->block_operands()) {
        if (target == phi_block)
          editor()->SetBlockOperand(last, position, new_block);
        ++position;
      }
      editor()->Commit();

      editor()->Edit(phi_block);
      editor()->ReplacePhiInputs(new_block, predecessor);
      editor()->Commit();
    }
  }
}

}  // namespace lir
}  // namespace elang
