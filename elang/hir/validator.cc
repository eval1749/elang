// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "elang/hir/validator.h"

#include "base/macros.h"
#include "elang/hir/editor.h"
#include "elang/hir/error_code.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Validator
//
Validator::Validator(Editor* editor) : editor_(editor), is_valid_(false) {
}

Validator::~Validator() {
}

void Validator::Error(ErrorCode error_code, const Value* error_value) {
  is_valid_ = false;
  editor()->Error(error_code, error_value);
}

void Validator::Error(ErrorCode error_code, const Value* value, Thing* detail) {
  is_valid_ = false;
  editor()->Error(error_code, value, detail);
}

void Validator::Error(ErrorCode error_code,
                      const Value* error_value,
                      const std::vector<Thing*>& details) {
  is_valid_ = false;
  editor()->Error(error_code, error_value, details);
}

void Validator::Error(ErrorCode error_code,
                      const Instruction* instruction,
                      int index) {
  is_valid_ = false;
  editor()->Error(error_code, instruction, {NewInt32(index)});
}

void Validator::Error(ErrorCode error_code,
                      const Instruction* instruction,
                      int index,
                      Thing* detail) {
  is_valid_ = false;
  editor()->Error(error_code, instruction, {NewInt32(index), detail});
}

Value* Validator::NewInt32(int32_t data) {
  return editor()->NewInt32(data);
}

// Validates |BasicBlock|
//  - id() in list
//  - function() in list
//  - terminator at the last
//  - instructions
bool Validator::Validate(BasicBlock* block) {
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

  auto const entry_block = block->function()->entry_block();
  if (block->first_instruction()->is<EntryInstruction>() &&
      entry_block != block) {
    Error(ErrorCode::ValidateBasicBlockEntry, block, entry_block);
    return false;
  }

  auto const exit_block = block->function()->exit_block();
  if (block->first_instruction()->is<ExitInstruction>() &&
      exit_block != block) {
    Error(ErrorCode::ValidateBasicBlockExit, block, exit_block);
    return false;
  }

  // Check instructions
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
    if (!Validate(instruction))
      is_valid = false;
  }
  if (!found_terminator) {
    Error(ErrorCode::ValidateBasicBlockNoTerminator, block);
    return false;
  }
  return is_valid;
}

bool Validator::Validate(Function* function) {
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

bool Validator::Validate(Instruction* instruction) {
  is_valid_ = true;
  instruction->Accept(this);
  return is_valid_;
}

// InstructionVisitor
void Validator::VisitBranch(BranchInstruction* instr) {
  if (!instr->output_type()->is<VoidType>()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }

  if (instr->IsUnconditionalBranch()) {
    if (!instr->operand(0)->is<BasicBlock>()) {
      Error(ErrorCode::ValidateInstructionOperand, instr, 0);
    }
    return;
  }

  DCHECK(instr->IsConditionalBranch());
  if (!instr->operand(0)->type()->is<BoolType>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0);
    return;
  }
  if (!instr->operand(1)->is<BasicBlock>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1);
    return;
  }
  if (!instr->operand(2)->is<BasicBlock>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 2);
    return;
  }
}

void Validator::VisitCall(CallInstruction* instr) {
  auto const function_type = instr->operand(0)->type()->as<FunctionType>();
  if (!function_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0);
    return;
  }
  if (instr->output_type() != function_type->return_type()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }
  if (instr->operand(1)->type() != function_type->parameters_type()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1);
    return;
  }
}

void Validator::VisitEntry(EntryInstruction* instr) {
  if (instr->output_type() != instr->function()->parameters_type()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }
  return;
}

void Validator::VisitExit(ExitInstruction* instr) {
  if (!instr->output_type()->is<VoidType>()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }
}

void Validator::VisitLoad(LoadInstruction* instr) {
  auto const pointer_type = instr->operand(0)->type()->as<PointerType>();
  if (!pointer_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0,
          instr->operand(0)->type());
    return;
  }
  if (instr->output_type() != pointer_type->pointee()) {
    Error(ErrorCode::ValidateInstructionOutput, instr, pointer_type);
    return;
  }
}

void Validator::VisitPhi(PhiInstruction* instr) {
  for (auto const predecessor : instr->basic_block()->predecessors()) {
    if (!instr->FindPhiInputFor(predecessor)) {
      Error(ErrorCode::ValidatePhiNotFound, instr, predecessor);
      return;
    }
  }
  // TODO(eval1749) We should check type of `phi` operands are subtype of
  // output type.
  auto position = 0;
  for (auto const input : instr->inputs()) {
    if (input->value()->type() != instr->output_type()) {
      Error(ErrorCode::ValidateInstructionOperand, instr, position,
            input->value());
      return;
    }
    ++position;
  }
  if (!position) {
    Error(ErrorCode::ValidatePhiCount, instr);
    return;
  }
  if (position == 1)
    Error(ErrorCode::ValidatePhiOne, instr);
}

void Validator::VisitReturn(ReturnInstruction* instr) {
  auto const return_type = instr->function()->return_type();
  if (instr->operand(0)->type() != return_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr,
          {NewInt32(0), return_type});
    return;
  }
  auto const exit_block = instr->function()->exit_block();
  if (instr->operand(1) != exit_block) {
    Error(ErrorCode::ValidateInstructionOperand, instr,
          {NewInt32(1), exit_block});
    return;
  }
}

void Validator::VisitStore(StoreInstruction* instr) {
  if (!instr->output_type()->is<VoidType>()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }
  auto const pointer_type = instr->operand(0)->type()->as<PointerType>();
  if (!pointer_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0, pointer_type);
    return;
  }
  // TODO(eval1749) We should check type of instr->operand(2) is subtype of
  // |pointer_type->pointee()|.
  auto const pointee = pointer_type->pointee();
  if (instr->operand(1)->type() != pointee) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1, pointee);
    return;
  }
}

}  // namespace hir
}  // namespace elang
