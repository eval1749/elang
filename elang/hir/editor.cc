// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "elang/hir/editor.h"

#include "base/logging.h"
#include "elang/hir/error_code.h"
#include "elang/hir/error_data.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"

namespace std {
ostream& operator<<(ostream& ostream, vector<elang::hir::ErrorData*> errors) {
  for (auto const error : errors)
    ostream << *error << std::endl;
  return ostream;
}
}  // namespace std

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : ZoneUser(factory->zone()), factory_(factory), function_(function) {
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

void Editor::Error(ErrorCode error_code, const Value* error_value) {
  Error(error_code, error_value, std::vector<Thing*>{});
}

void Editor::Error(ErrorCode error_code, const Value* value, Thing* detail) {
  Error(error_code, value, std::vector<Thing*>{detail});
}

void Editor::Error(ErrorCode error_code,
                   const Value* error_value,
                   const std::vector<Thing*>& details) {
  errors_.push_back(new (zone()) ErrorData(
      zone(), error_code, const_cast<Value*>(error_value), details));
}

void Editor::Error(ErrorCode error_code,
                   const Instruction* instruction,
                   int index) {
  Error(error_code, instruction, {NewInt32(index)});
}

void Editor::Error(ErrorCode error_code,
                   const Instruction* instruction,
                   int index,
                   Thing* detail) {
  Error(error_code, instruction, {NewInt32(index), detail});
}

bool Editor::Commit() {
  DCHECK(!basic_blocks_.empty());
  auto succeeded = true;
  for (auto const basic_block : basic_blocks_) {
    if (!Validate(basic_block))
      succeeded = false;
  }
  basic_blocks_.clear();
  return succeeded;
}

void Editor::Edit(BasicBlock* basic_block) {
  DCHECK(std::find(basic_blocks_.begin(), basic_blocks_.end(), basic_block) ==
         basic_blocks_.end());
  basic_blocks_.push_back(basic_block);
  if (basic_block->instructions_.empty())
    return;
  DCHECK(Validate(basic_block)) << errors();
}

void Editor::InitializeFunctionIfNeeded() {
  if (!function_->basic_blocks_.empty()) {
    DCHECK(Validate(function_)) << errors();
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
    SetReturn(function_->return_type()->default_value());
  }

  DCHECK(Validate(function_)) << errors();
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

Value* Editor::NewInt32(int32_t data) {
  return factory()->NewInt32Literal(data);
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

void Editor::SetBranch(BasicBlock* target_block) {
  SetTerminator(factory()->NewBranchInstruction(target_block));
}

void Editor::SetInput(Instruction* instruction, int index, Value* new_value) {
  instruction->SetOperandAt(index, new_value);
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

// Validates |BasicBlock|
//  - id() in list
//  - function() in list
//  - terminator at the last
//  - instructions
bool Editor::Validate(BasicBlock* block) {
  if (!block->id()) {
    Error(ErrorCode::ValidateBasicBlockNoId, block);
    return false;
  }
  if (!block->function()) {
    Error(ErrorCode::ValidateBasicBlockNoFunction, block);
    return false;
  }
  if (block->instructions().empty()) {
    Error(ErrorCode::ValidateBasicBlockEmpty, block);
    return false;
  }
  auto found_terminator = false;
  auto is_valid = true;
  for (auto instruction : block->instructions()) {
    if (!instruction->id()) {
      Error(ErrorCode::ValidateInstructionNoId, instruction);
      return false;
    }
    if (instruction->IsTerminator()) {
      if (found_terminator) {
        Error(ErrorCode::ValidateInstructionTerminator, instruction);
        return false;
      }
      found_terminator = true;
    }
    if (!instruction->Validate(this))
      is_valid = false;
  }
  if (!found_terminator) {
    Error(ErrorCode::ValidateBasicBlockNoTerminator, block);
    return false;
  }
  return true;
}

bool Editor::Validate(Function* function) {
  if (function->basic_blocks().empty()) {
    Error(ErrorCode::ValidateFunctionEmpty, function);
    return false;
  }
  if (!function->entry_block()->first_instruction()->is<EntryInstruction>()) {
    Error(ErrorCode::ValidateFunctionNoEntry, function);
    return false;
  }
  auto found_exit = false;
  for (auto block : function->basic_blocks()) {
    if (!block->id()) {
      Error(ErrorCode::ValidateBasicBlockNoId, block);
      return false;
    }
    if (!Validate(block))
      return false;
    if (block->last_instruction()->is<ExitInstruction>()) {
      if (found_exit) {
        Error(ErrorCode::ValidateFunctionExit, function);
        return false;
      }
      found_exit = true;
    }
  }
  if (!found_exit) {
    Error(ErrorCode::ValidateFunctionNoExit, function);
    return false;
  }
  return true;
}

}  // namespace hir
}  // namespace elang
