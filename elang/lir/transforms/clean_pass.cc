// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/clean_pass.h"

#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

CleanPass::CleanPass(Editor* editor) : FunctionPass(editor), changed_(false) {
}

CleanPass::~CleanPass() {
}

void CleanPass::Clean() {
  std::vector<BasicBlock*> blocks(editor()->PostOrderList().begin(),
                                  editor()->PostOrderList().end());
  // Process successors before processing |block|.
  for (auto block : blocks) {
    DCHECK(!block->predecessors().empty() ||
           block->first_instruction()->opcode() == Opcode::Entry);
    auto const instr = block->last_instruction();
    if (auto const branch = instr->as<BranchInstruction>()) {
      CleanBranch(branch);
      continue;
    }
    if (auto const jump = instr->as<JumpInstruction>()) {
      CleanJump(jump);
      continue;
    }
  }
  DCHECK(editor()->Validate()) << *editor();
}

void CleanPass::CleanBranch(BranchInstruction* branch) {
  auto const false_block = branch->false_block();
  auto const false_instr = false_block->first_instruction();
  if (false_instr->opcode() != Opcode::Jump)
    return;
  auto const true_block = branch->true_block();
  auto const true_instr = true_block->first_instruction();
  if (true_instr->opcode() != false_instr->opcode())
    return;

  auto const target_block = false_instr->block_operand(0);
  if (true_instr->block_operand(0) != target_block)
    return;

  if (!target_block->phi_instructions().empty())
    return;

  editor()->Edit(branch->basic_block());
  WillChangeControlFlow("Fold a redundant branch", branch);
  editor()->SetJump(target_block);
  auto const jump =
      editor()->basic_block()->last_instruction()->as<JumpInstruction>();
  DidChangeControlFlow("Fold a redundant branch", jump);
  editor()->Commit();

  CleanJump(jump);
}

void CleanPass::CleanJump(JumpInstruction* jump) {
  auto const block = jump->basic_block();
  auto const target = jump->target_block();
  if (block->first_instruction() == jump &&
      target->phi_instructions().empty()) {
    WillChangeControlFlow("Remove an empty block", jump);
    std::vector<BasicBlock*> predecessors(block->predecessors().begin(),
                                          block->predecessors().end());
    editor()->DiscardBlock(block);
    for (auto const predecessor : predecessors) {
      editor()->Edit(predecessor);
      auto const last_instr = predecessor->last_instruction();
      auto position = 0;
      for (auto const operand : last_instr->block_operands()) {
        if (operand == block) {
          editor()->SetBlockOperand(last_instr, position, target);
        }
        ++position;
      }
      editor()->Commit();
    }
    DidChangeControlFlow("Remove an empty block", target->first_instruction());
    return;
  }

  if (target->predecessors().size() == 1u &&
      target->phi_instructions().empty()) {
    WillChangeControlFlow("Combine blocks", jump);
    editor()->Edit(block);
    editor()->CombineBlock(target);
    editor()->Commit();
    editor()->DiscardBlock(target);
    DidChangeControlFlow("Combine blocks", block->last_instruction());
    return;
  }

  auto const branch = target->first_instruction()->as<BranchInstruction>();
  if (!branch)
    return;
  if (!branch->true_block()->phi_instructions().empty() ||
      !branch->false_block()->phi_instructions().empty()) {
    return;
  }

  WillChangeControlFlow("Hoist a branch", branch);
  editor()->Edit(block);
  editor()->SetBranch(branch->input(0), branch->true_block(),
                      branch->false_block());
  editor()->Commit();
  DidChangeControlFlow("Hoist a branch", block->last_instruction());
}

void CleanPass::DidChangeControlFlow(base::StringPiece message,
                                     const Instruction* instr) {
  DVLOG(1) << "After " << message << ": " << *instr;
  changed_ = true;
}

void CleanPass::WillChangeControlFlow(base::StringPiece message,
                                      const Instruction* instr) {
  DVLOG(1) << "Before " << message << ": " << *instr;
  changed_ = true;
}

// Pass
base::StringPiece CleanPass::name() const {
  return "lir_clean";
}

// FunctionPass
void CleanPass::RunOnFunction() {
  do {
    changed_ = false;
    Clean();
  } while (changed_);
}

}  // namespace lir
}  // namespace elang
