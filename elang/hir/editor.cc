// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/editor.h"

#include "base/logging.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : factory_(factory), function_(function) {
  InitializeFunctionIfNeeded();
}

Editor::~Editor() {
  DCHECK(basic_blocks_.empty());
}

BasicBlock* Editor::entry_block() const {
  return function_->entry_block();
}

BasicBlock* Editor::exit_block() const {
  return function_->exit_block();
}

void Editor::Append(Instruction* new_instruction) {
  DCHECK(!new_instruction->basic_block_);
  DCHECK(!new_instruction->id_);
  DCHECK(!basic_blocks_.empty());
  auto const basic_block = basic_blocks_.back();
  auto const last = basic_block->last_instruction();
  if (last && last->IsTerminator())
    basic_block->instructions_.InsertBefore(new_instruction, last);
  else
    basic_block->instructions_.AppendNode(new_instruction);
  new_instruction->id_ = factory()->NextInstructionId();
  new_instruction->basic_block_ = basic_block;
}

void Editor::Commit() {
  DCHECK(!basic_blocks_.empty());
  for (auto const basic_block : basic_blocks_) {
    DCHECK(Validate(basic_block));
  }
  basic_blocks_.clear();
}

void Editor::Edit(BasicBlock* basic_block) {
  DCHECK(std::find(basic_blocks_.begin(), basic_blocks_.end(), basic_block) ==
         basic_blocks_.end());
  basic_blocks_.push_back(basic_block);
  if (basic_block->instructions_.empty())
    return;
  DCHECK(Validate(basic_block));
}

void Editor::InitializeFunctionIfNeeded() {
  if (!function_->basic_blocks_.empty()) {
    DCHECK(Validate(function_));
    return;
  }

  // Make entry and exit block
  auto const entry = factory()->NewBasicBlock();
  function_->basic_blocks_.AppendNode(entry);
  entry->function_ = function_;
  entry->id_ = factory()->NextBasicBlockId();

  auto const exit = factory()->NewBasicBlock();
  function_->basic_blocks_.AppendNode(exit);
  exit->function_ = function_;
  exit->id_ = factory()->NextBasicBlockId();

  {
    ScopedEdit edit_scope(this);

    // Since 'ret' instruction refers exit block, we create exit block before
    // entry block.
    Edit(exit);
    Append(factory()->NewExitInstruction());

    Edit(entry);
    Append(factory()->NewEntryInstruction(
        function_->function_type()->parameters_type()));
    SetReturn(function_->function_type()->return_type()->GetDefaultValue());
  }

  DCHECK(Validate(function_));
}

void Editor::InsertBefore(Instruction* new_instruction,
                          Instruction* ref_instruction) {
  if (!ref_instruction) {
    Append(new_instruction);
    return;
  }
  DCHECK(!basic_blocks_.empty());
  auto const basic_block = basic_blocks_.back();
  DCHECK_EQ(basic_block, ref_instruction->basic_block());
  DCHECK(!new_instruction->basic_block_);
  DCHECK(!new_instruction->id_);
  basic_block->instructions_.InsertBefore(new_instruction, ref_instruction);
  new_instruction->id_ = factory()->NextInstructionId();
  new_instruction->basic_block_ = basic_block;
}

BasicBlock* Editor::NewBasicBlock() {
  auto const new_basic_block = factory()->NewBasicBlock();
  basic_blocks_.push_back(new_basic_block);
  new_basic_block->function_ = function_;
  new_basic_block->id_ = factory()->NextBasicBlockId();
  // We keep exit block at end of basic block list.
  function_->basic_blocks_.InsertBefore(new_basic_block, exit_block());
  return new_basic_block;
}

void Editor::RemoveInstruction(Instruction* old_instruction) {
  DCHECK(!basic_blocks_.empty());
  // We should update use-def list for values used by |old_instruction|.
  auto const operand_count = old_instruction->CountOperands();
  for (auto index = 0; index < operand_count; ++index)
    old_instruction->ResetOperandAt(index);
  auto const basic_block = basic_blocks_.back();
  basic_block->instructions_.RemoveNode(old_instruction);
  // Mark |old_instruction| is removed from tree.
  old_instruction->id_ = 0;
  old_instruction->basic_block_ = nullptr;
}

void Editor::SetBranch(Value* condition,
                       BasicBlock* true_block,
                       BasicBlock* false_block) {
  DCHECK(!basic_blocks_.empty());
  SetTerminator(
      factory()->NewBranchInstruction(condition, true_block, false_block));
}

void Editor::SetInput(Instruction* instruction, int index, Value* new_value) {
  instruction->SetOperandAt(index, new_value);
}

void Editor::SetJump(BasicBlock* target_block) {
  SetTerminator(factory()->NewJumpInstruction(target_block));
}

void Editor::SetReturn(Value* new_value) {
  DCHECK(!basic_blocks_.empty());
  SetTerminator(factory()->NewReturnInstruction(new_value, exit_block()));
}

void Editor::SetTerminator(Instruction* terminator) {
  DCHECK(terminator->IsTerminator());
  auto const basic_block = basic_blocks_.back();
  auto const last = basic_block->last_instruction();
  if (last && last->IsTerminator())
    RemoveInstruction(last);
  Append(terminator);
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
    if (auto const index = instruction->ValidateOperands()) {
      DVLOG(0) << "Operand[" << index + 1
               << "]=" << instruction->OperandAt(index) << " is invalid.";
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

}  // namespace hir
}  // namespace elang
