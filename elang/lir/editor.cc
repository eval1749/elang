// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/lir/editor.h"

#include "base/logging.h"
#include "elang/lir/error_data.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/validator.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : basic_block_(nullptr), factory_(factory), function_(function) {
}

Editor::~Editor() {
  DCHECK(!basic_block_);
}

void Editor::Append(Instruction* new_instruction) {
  DCHECK(!new_instruction->basic_block_);
  DCHECK(!new_instruction->id_);
  DCHECK(basic_block_);
  auto const last = basic_block_->last_instruction();
  if (last && last->IsTerminator())
    basic_block_->instructions_.InsertBefore(new_instruction, last);
  else
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

void Editor::AddError(ErrorCode error_code,
                      Value value,
                      const std::vector<Value> details) {
  errors_.push_back(new (factory()->zone()) ErrorData(
      factory()->zone(), factory()->literals(), error_code, value, details));
}

void Editor::Error(ErrorCode error_code, Value value) {
  AddError(error_code, value, {});
}

void Editor::Error(ErrorCode error_code, Value value, Value detail) {
  AddError(error_code, value, {detail});
}

void Editor::Error(ErrorCode error_code,
                   Value value,
                   Value detail1,
                   Value detail2) {
  AddError(error_code, value, {detail1, detail2});
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

PhiInstruction* Editor::NewPhi(Value output) {
  DCHECK(basic_block_);
  auto const phi_instruction = factory()->NewPhiInstruction(output);
  basic_block_->phi_instructions_.AppendNode(phi_instruction);
  return phi_instruction->as<PhiInstruction>();
}

void Editor::Remove(Instruction* old_instruction) {
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, old_instruction->basic_block_);
  basic_block_->instructions_.RemoveNode(old_instruction);
  old_instruction->id_ = 0;
  old_instruction->basic_block_ = nullptr;
}

void Editor::SetBranch(Value condition,
                       BasicBlock* true_block,
                       BasicBlock* false_block) {
  DCHECK(basic_block_);
  DCHECK(false_block->id());
  DCHECK(true_block->id());
  if (auto const last =
          basic_block_->last_instruction()->as<BranchInstruction>()) {
    SetInput(last, 0, condition);
    SetInput(last, 1, true_block->value());
    SetInput(last, 2, false_block->value());
    return;
  }
  SetTerminator(
      factory()->NewBranchInstruction(condition, true_block, false_block));
}

void Editor::SetInput(Instruction* instruction, int index, Value new_value) {
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, instruction->basic_block());
  instruction->SetInput(index, new_value);
}

void Editor::SetJump(BasicBlock* target_block) {
  DCHECK(basic_block_);
  if (auto const last =
          basic_block_->last_instruction()->as<JumpInstruction>()) {
    SetInput(last, 0, target_block->value());
    return;
  }
  SetTerminator(factory()->NewJumpInstruction(target_block));
}

void Editor::SetPhiInput(PhiInstruction* phi,
                         BasicBlock* block,
                         Value new_value) {
  if (auto const present = phi->FindPhiInputFor(block)) {
    present->value_ = new_value;
    return;
  }
  auto const new_input = new (factory()->zone()) PhiInput(block, new_value);
  phi->phi_inputs_.AppendNode(new_input);
}

void Editor::SetOutput(Instruction* instruction, int index, Value new_value) {
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, instruction->basic_block());
  instruction->SetOutput(index, new_value);
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

bool Editor::Validate() {
  Validator validator(this);
  return validator.Validate(function_);
}

bool Editor::Validate(BasicBlock* block) {
  Validator validator(this);
  return validator.Validate(block);
}

bool Editor::Validate(Function* function) {
  Validator validator(this);
  return validator.Validate(function);
}

}  // namespace lir
}  // namespace elang
