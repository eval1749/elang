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
#include "elang/hir/validator.h"

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
// Editor::ScopedEdit
//
Editor::ScopedEdit::ScopedEdit(Editor* editor, BasicBlock* basic_block)
    : editor_(editor) {
  editor->Edit(basic_block);
}

Editor::ScopedEdit::~ScopedEdit() {
  editor_->Commit();
}

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : ZoneUser(factory->zone()),
      basic_block_(nullptr),
      factory_(factory),
      function_(function),
      validator_(new Validator(this)) {
  InitializeFunctionIfNeeded();
}

Editor::~Editor() {
  DCHECK(!basic_block_);
}

BasicBlock* Editor::entry_block() const {
  return function_->entry_block();
}

BasicBlock* Editor::exit_block() const {
  return function_->exit_block();
}

void Editor::Append(Instruction* new_instruction) {
  DCHECK(!new_instruction->is<PhiInstruction>());
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
  auto const is_valid = Validate(basic_block_);
  basic_block_ = nullptr;
  return is_valid;
}

void Editor::Continue(BasicBlock* basic_block) {
  DCHECK(!basic_block_);
  basic_block_ = basic_block;
}

void Editor::Edit(BasicBlock* basic_block) {
  DCHECK(!basic_block_);
  basic_block_ = basic_block;
  if (basic_block_->instructions().empty())
    return;
  DCHECK(Validate(basic_block)) << errors();
}

BasicBlock* Editor::EditNewBasicBlock(BasicBlock* reference) {
  auto const new_basic_block = NewBasicBlock(reference);
  Edit(new_basic_block);
  return new_basic_block;
}

BasicBlock* Editor::EditNewBasicBlock() {
  return EditNewBasicBlock(exit_block());
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

void Editor::InitializeFunctionIfNeeded() {
  if (!function_->basic_blocks_.empty()) {
    DCHECK(Validate(function_)) << errors();
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
  Append(factory()->NewEntryInstruction(
      function_->function_type()->parameters_type()));
  SetReturn(function_->return_type()->default_value());

  basic_block_ = nullptr;
  DCHECK(Validate(function_)) << errors();
}

void Editor::InsertBefore(Instruction* new_instruction,
                          Instruction* ref_instruction) {
  DCHECK(!new_instruction->is<PhiInstruction>());
  DCHECK(!ref_instruction->is<PhiInstruction>());
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
  auto const new_basic_block = factory()->NewBasicBlock();
  new_basic_block->function_ = function_;
  new_basic_block->id_ = factory()->NextBasicBlockId();
  // We keep exit block at end of basic block list.
  function_->basic_blocks_.InsertBefore(new_basic_block, reference);
  return new_basic_block;
}

Value* Editor::NewInt32(int32_t data) {
  return factory()->NewInt32Literal(data);
}

PhiInstruction* Editor::NewPhi(Type* output_type) {
  DCHECK(basic_block_);
  auto const phi = factory()->NewPhiInstruction(output_type);
  basic_block_->phi_instructions_.AppendNode(phi);
  phi->id_ = factory()->NextInstructionId();
  phi->basic_block_ = basic_block_;
  return phi;
}

void Editor::RemoveInstruction(Instruction* old_instruction) {
  DCHECK(basic_block_);
  ResetInputs(old_instruction);
  basic_block_->instructions_.RemoveNode(old_instruction);
  // Mark |old_instruction| is removed from tree.
  old_instruction->id_ = 0;
  old_instruction->basic_block_ = nullptr;
}

void Editor::ResetInputs(Instruction* instruction) {
  // We should update use-def list for values used by |instruction|.
  auto const operand_count = instruction->CountOperands();
  for (auto index = 0; index < operand_count; ++index)
    instruction->ResetOperandAt(index);
}

void Editor::SetBranch(Value* condition,
                       BasicBlock* true_block,
                       BasicBlock* false_block) {
  DCHECK(basic_block_);
  DCHECK(!condition->is<BasicBlock>());
  if (auto const branch =
          basic_block_->last_instruction()->as<BranchInstruction>()) {
    branch->SetOperandAt(0, condition);
    branch->SetOperandAt(1, true_block);
    branch->SetOperandAt(2, true_block);
    return;
  }
  SetTerminator(
      factory()->NewBranchInstruction(condition, true_block, false_block));
}

void Editor::SetBranch(BasicBlock* target_block) {
  DCHECK(basic_block_);
  if (auto const branch =
          basic_block_->last_instruction()->as<BranchInstruction>()) {
    ResetInputs(branch);
    branch->SetOperandAt(0, target_block);
    return;
  }
  SetTerminator(factory()->NewBranchInstruction(target_block));
}

void Editor::SetInput(Instruction* instruction, int index, Value* new_value) {
  DCHECK(basic_block_);
  DCHECK_EQ(instruction->basic_block(), basic_block_);
  instruction->SetOperandAt(index, new_value);
}

void Editor::SetPhiInput(PhiInstruction* phi, BasicBlock* block, Value* value) {
  if (auto const present = phi->FindPhiInputFor(block)) {
    present->SetValue(value);
    return;
  }
  auto const new_input = new (zone()) PhiInput(phi, block, value);
  phi->inputs_.AppendNode(new_input);
}

void Editor::SetReturn(Value* new_value) {
  DCHECK(basic_block_);
  SetTerminator(factory()->NewReturnInstruction(new_value, exit_block()));
}

void Editor::SetTerminator(Instruction* terminator) {
  DCHECK(terminator->IsTerminator());
  auto const last = basic_block_->last_instruction();
  if (last && last->IsTerminator())
    RemoveInstruction(last);
  Append(terminator);
}

BasicBlock* Editor::SplitBefore(Instruction* reference) {
  DCHECK(!basic_block_);
  auto const ref_basic_block = reference->basic_block();
  DCHECK(!reference->is<PhiInstruction>());
  auto const new_basic_block = NewBasicBlock(ref_basic_block->next());
  for (auto runner = reference; runner; runner = runner->next()) {
    ref_basic_block->instructions_.RemoveNode(runner);
    new_basic_block->instructions_.AppendNode(runner);
  }
  DCHECK(Validate(new_basic_block)) << errors_;
  return new_basic_block;
}

bool Editor::Validate(BasicBlock* block) {
  return validator_->Validate(block);
}

bool Editor::Validate(Function* function) {
  return validator_->Validate(function);
}

}  // namespace hir
}  // namespace elang
