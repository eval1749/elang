// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/lir/editor.h"

#include "base/logging.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : basic_block_(nullptr), factory_(factory), function_(function) {
  InitializeFunctionIfNeeded();
}

Editor::~Editor() {
  DCHECK(!basic_block_);
}

void Editor::Append(Instruction* new_instruction) {
  DCHECK(!new_instruction->basic_block_);
  DCHECK(!new_instruction->id_);
  DCHECK(basic_block_);
  basic_block_->instructions_.AppendNode(new_instruction);
  new_instruction->id_ = factory()->NextInstructionId();
  new_instruction->basic_block_ = basic_block_;
}

bool Editor::Commit() {
  DCHECK(basic_block_);
#ifndef NDEBUG
  basic_block_ = nullptr;
  return true;
#else
  auto const is_valid = Validate(basic_block_);
  basic_block_ = nullptr;
  return is_valid;
#endif
}

void Editor::Edit(BasicBlock* basic_block) {
  DCHECK(!basic_block_);
  DCHECK_EQ(function(), basic_block->function());
  basic_block_ = basic_block;
  if (basic_block_->instructions().empty())
    return;
  DCHECK(Validate(basic_block_));
}

void Editor::EditNewBasicBlock() {
  Edit(NewBasicBlock(function()->exit_block()));
}

void Editor::InitializeFunctionIfNeeded() {
  if (!function()->basic_blocks_.empty()) {
    DCHECK(Validate(function()));
    return;
  }

  // Make entry and exit block
  // Since |Validator| uses entry and exit blocks, we can't use editing
  // functions for populating entry and exit block.
  auto const entry_block = factory()->NewBasicBlock();
  function_->basic_blocks_.AppendNode(entry_block);
  entry_block->function_ = function_;
  entry_block->id_ = factory()->NextBasicBlockId();

  auto const exit_block = factory()->NewBasicBlock();
  function_->basic_blocks_.AppendNode(exit_block);
  exit_block->function_ = function_;
  exit_block->id_ = factory()->NextBasicBlockId();

  basic_block_ = exit_block;
  Append(factory()->NewExitInstruction());

  basic_block_ = entry_block;
  Append(factory()->NewEntryInstruction());
  Append(factory()->NewRetInstruction());

  basic_block_ = nullptr;
  DCHECK(Validate(function()));
}

void Editor::InsertBefore(Instruction* new_instruction,
                          Instruction* ref_instruction) {
  if (!ref_instruction) {
    Append(new_instruction);
    return;
  }
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, ref_instruction->basic_block());
  DCHECK(!new_instruction->basic_block_);
  DCHECK(!new_instruction->id_);
  basic_block_->instructions_.InsertBefore(new_instruction, ref_instruction);
  new_instruction->id_ = factory()->NextInstructionId();
  new_instruction->basic_block_ = basic_block_;
}

BasicBlock* Editor::NewBasicBlock(BasicBlock* reference) {
  DCHECK(reference);
  DCHECK_EQ(function(), reference->function());
  auto const new_block = factory()->NewBasicBlock();
  new_block->function_ = function();
  new_block->id_ = factory()->NextBasicBlockId();
  // We keep exit block at end of basic block list.
  function()->basic_blocks_.InsertBefore(new_block, reference);
  return new_block;
}

void Editor::Remove(Instruction* old_instruction) {
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, old_instruction->basic_block_);
  basic_block_->instructions_.RemoveNode(old_instruction);
  old_instruction->id_ = 0;
  old_instruction->basic_block_ = nullptr;
}

void Editor::SetInput(Instruction* instruction, int index, Value new_value) {
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, instruction->basic_block());
  instruction->inputs_[index] = new_value;
}

void Editor::SetJump(BasicBlock* target_block) {
  DCHECK(basic_block_);
  if (auto const last =
          basic_block_->last_instruction()->as<JumpInstruction>()) {
    SetInput(last, 0, target_block->value());
    return;
  }
  auto const instr = factory()->NewJumpInstruction();
  SetTerminator(instr);
  SetInput(instr, 0, target_block->value());
}

void Editor::SetOutput(Instruction* instruction, int index, Value new_value) {
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, instruction->basic_block());
  instruction->outputs_[index] = new_value;
}

void Editor::SetReturn() {
  DCHECK(basic_block_);
  if (auto const last = basic_block_->last_instruction()->as<RetInstruction>())
    return;
  SetTerminator(factory()->NewRetInstruction());
}

void Editor::SetTerminator(Instruction* instr) {
  DCHECK(basic_block_);
  DCHECK(!instr->basic_block_);
  DCHECK(instr->IsTerminator());
  auto const last = basic_block_->last_instruction();
  if (last && last->IsTerminator())
    Remove(last);
  Append(instr);
}

bool Editor::Validate(BasicBlock* block) {
  if (!block->id()) {
    DVLOG(0) << *block << " should have id.";
    return false;
  }
  if (!block->function()) {
    DVLOG(0) << *block << " is orphan.";
    return false;
  }
  if (block->instructions().empty()) {
    DVLOG(0) << *block << " is empty.";
    return false;
  }
  auto found_terminator = false;
  for (auto instruction : block->instructions()) {
    // TODO(eval1749) We should call |Validation()| function in ISA.
    if (!instruction->id()) {
      DVLOG(0) << *instruction << " should have an id.";
      return false;
    }
    if (instruction->IsTerminator()) {
      if (found_terminator) {
        DVLOG(0) << *block << " has " << *instruction << " at middle.";
        return false;
      }
      found_terminator = true;
    }
  }
  if (!found_terminator) {
    DVLOG(0) << *block << " should have terminator instruction"
             << " instead of " << *block->last_instruction();
    return false;
  }
  return true;
}

bool Editor::Validate(Function* function) {
  if (function->basic_blocks().empty()) {
    DVLOG(0) << *function << " should have blocks.";
    return false;
  }
  if (!function->entry_block()->first_instruction()->is<EntryInstruction>()) {
    DVLOG(0) << *function << " should have an entry block.";
    return false;
  }
  auto found_exit = false;
  for (auto block : function->basic_blocks()) {
    if (!block->id()) {
      DVLOG(0) << *block << " should have an id.";
      return false;
    }
    if (!Validate(block))
      return false;
    if (block->last_instruction()->is<ExitInstruction>()) {
      if (found_exit) {
        DVLOG(0) << *function << " should have only one exit block.";
        return false;
      }
      found_exit = true;
    }
  }
  if (!found_exit) {
    DVLOG(0) << *function << " should have an exit block.";
    return false;
  }
  return true;
}

}  // namespace lir
}  // namespace elang
