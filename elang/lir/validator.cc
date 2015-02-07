// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/lir/validator.h"

#include "base/logging.h"
#include "elang/lir/editor.h"
#include "elang/lir/error_code.h"
#include "elang/lir/error_data.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Validator
//
Validator::Validator(Editor* editor)
    : editor_(editor), is_valid_instruction_(false) {
}

Validator::~Validator() {
}

BasicBlock* Validator::entry_block() const {
  return function()->entry_block();
}

BasicBlock* Validator::exit_block() const {
  return function()->exit_block();
}

Function* Validator::function() const {
  return editor()->function();
}

void Validator::AddError(ErrorCode error_code,
                         Value value,
                         const std::vector<Value> details) {
  if (value.is_instruction())
    is_valid_instruction_ = false;
  editor_->AddError(error_code, value, details);
}

Value Validator::AsValue(Instruction* instruction) {
  return editor_->factory()->literals()->RegisterInstruction(instruction);
}

void Validator::Error(ErrorCode error_code, Instruction* instruction) {
  Error(error_code, AsValue(instruction));
}

void Validator::Error(ErrorCode error_code, Instruction* instruction,
                      int detail) {
  Error(error_code, AsValue(instruction),
        editor_->factory()->NewIntValue(Value::Size::Size32, detail));
}

void Validator::Error(ErrorCode error_code, Instruction* instruction,
                      Value detail) {
  Error(error_code, AsValue(instruction), detail);
}

void Validator::Error(ErrorCode error_code, Value value) {
  AddError(error_code, value, {});
}

void Validator::Error(ErrorCode error_code, Value value, Value detail) {
  AddError(error_code, value, {detail});
}

void Validator::Error(ErrorCode error_code,
                      Value value,
                      Value detail1,
                      Value detail2) {
  AddError(error_code, value, {detail1, detail2});
}

Literal* Validator::GetLiteral(Value value) {
  if (!value.is_literal())
    return nullptr;
  return editor()->factory()->literals()->GetLiteral(value);
}

bool Validator::Validate(BasicBlock* block) {
  if (!block->id()) {
    Error(ErrorCode::ValidateBasicBlockId, block->value());
    return false;
  }
  if (!block->function()) {
    Error(ErrorCode::ValidateBasicBlockFunction, block->value());
    return false;
  }
  if (block->instructions().empty()) {
    Error(ErrorCode::ValidateBasicBlockEmpty, block->value());
    return false;
  }

  // Entry block
  if (block == entry_block()) {
    if (!block->first_instruction()->is<EntryInstruction>()) {
      Error(ErrorCode::ValidateInstructionEntry, block->first_instruction());
      return false;
    }
    if (block->HasPredecessor()) {
      Error(ErrorCode::ValidateBasicBlockEntry, block->value());
      return false;
    }
  } else if (block->first_instruction()->is<EntryInstruction>()) {
    Error(ErrorCode::ValidateInstructionEntry, block->first_instruction());
    return false;
  }

  // Exit block
  if (block == exit_block()) {
    if (!block->first_instruction()->is<ExitInstruction>()) {
      Error(ErrorCode::ValidateInstructionExit, block->first_instruction());
      return false;
    }
    if (block->HasSuccessor()) {
      Error(ErrorCode::ValidateBasicBlockExit, block->value());
      return false;
    }
  } else if (block->last_instruction()->is<ExitInstruction>()) {
    Error(ErrorCode::ValidateInstructionExit, block->last_instruction());
    return false;
  }

  // Check instructions
  auto is_valid = true;
  auto found_terminator = false;
  for (auto instruction : block->instructions()) {
    if (!Validate(instruction))
      is_valid = false;
    if (instruction->IsTerminator()) {
      if (found_terminator) {
        Error(ErrorCode::ValidateInstructionTerminator, instruction);
        is_valid = false;
      }
      found_terminator = true;
    }
  }
  if (!found_terminator) {
    Error(ErrorCode::ValidateBasicBlockTerminator, block->value());
    return false;
  }
  return is_valid;
}

bool Validator::Validate(Function* function) {
  if (function->basic_blocks().empty()) {
    Error(ErrorCode::ValidateFunctionEmpty, function->value());
    return false;
  }
  if (!function->entry_block()->first_instruction()->is<EntryInstruction>()) {
    Error(ErrorCode::ValidateFunctionEntry, function->value());
    return false;
  }
  auto found_exit = false;
  auto is_valid = true;
  for (auto block : function->basic_blocks()) {
    if (!Validate(block)) {
      is_valid = false;
      continue;
    }
    if (block->last_instruction()->is<ExitInstruction>()) {
      if (found_exit) {
        Error(ErrorCode::ValidateBasicBlockExit, block->value());
        is_valid = false;
      }
      found_exit = true;
    }
  }
  if (!found_exit) {
    Error(ErrorCode::ValidateBasicBlockExit, function->value());
    is_valid = false;
  }
  return is_valid;
}

bool Validator::Validate(Instruction* instruction) {
  if (!instruction->id()) {
    Error(ErrorCode::ValidateInstructionId, instruction);
    return false;
  }
  if (!instruction->basic_block()) {
    Error(ErrorCode::ValidateInstructionBasicBlock, instruction);
    return false;
  }
  is_valid_instruction_ = true;
  instruction->Accept(this);
  return is_valid_instruction_;
}

// InstructionVisitor

void Validator::VisitBranch(BranchInstruction* instr) {
  if (!instr->input(0).is_condition())
    Error(ErrorCode::ValidateInstructionInput, instr, 0);
}

void Validator::VisitCopy(CopyInstruction* instr) {
  if (instr->output(0).size != instr->input(0).size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
  if (instr->output(0).type != instr->input(0).type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
}

void Validator::VisitRet(RetInstruction* instr) {
  if (instr->block_operand(0) != exit_block())
    Error(ErrorCode::ValidateInstructionSuccessor, instr, 0);
}

}  // namespace lir
}  // namespace elang
