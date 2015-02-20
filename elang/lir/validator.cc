// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <unordered_set>

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
Validator::Validator(Editor* editor) : editor_(editor) {
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
  editor_->AddError(error_code, value, details);
}

Value Validator::AsValue(Instruction* instruction) {
  return editor_->factory()->literals()->RegisterInstruction(instruction);
}

void Validator::Error(ErrorCode error_code, Instruction* instruction) {
  Error(error_code, AsValue(instruction));
}

void Validator::Error(ErrorCode error_code,
                      Instruction* instruction,
                      int detail) {
  Error(error_code, AsValue(instruction),
        editor_->factory()->NewIntValue(ValueSize::Size32, detail));
}

void Validator::Error(ErrorCode error_code,
                      Instruction* instruction,
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

  // Check edges
  for (auto const predecessor : block->predecessors()) {
    if (!function()->HasEdge(predecessor, block)) {
      Error(ErrorCode::ValidateBasicBlockPredecessor, block->value(),
            predecessor->value());
    }
  }

  for (auto const successor : block->successors()) {
    if (!function()->HasEdge(block, successor)) {
      Error(ErrorCode::ValidateBasicBlockSuccessor, block->value(),
            successor->value());
    }
  }

  // Check instructions
  auto found_terminator = false;
  for (auto const instruction : block->instructions()) {
    Validate(instruction);

    if (instruction->IsTerminator()) {
      if (found_terminator)
        Error(ErrorCode::ValidateInstructionTerminator, instruction);
      found_terminator = true;
    }
  }
  if (!found_terminator) {
    Error(ErrorCode::ValidateBasicBlockTerminator, block->value());
    return false;
  }
  return editor()->errors().empty();
}

bool Validator::Validate(Function* function) {
  if (function->basic_blocks().empty()) {
    Error(ErrorCode::ValidateFunctionEmpty, function->value());
    return false;
  }
  auto const entry_block = function->entry_block();
  if (!entry_block->first_instruction()->is<EntryInstruction>()) {
    Error(ErrorCode::ValidateFunctionEntry, function->value());
    return false;
  }
  auto const exit_block = function->exit_block();
  auto found_exit = false;
  for (auto block : function->basic_blocks()) {
    if (block != entry_block && !block->HasPredecessor())
      Error(ErrorCode::ValidateBasicBlockUnreachable, block->value());

    if (block != exit_block && !block->HasSuccessor())
      Error(ErrorCode::ValidateBasicBlockDeadEnd, block->value());

    // Check phi inputs
    {
      std::unordered_set<BasicBlock*> predecessors(
          block->predecessors().begin(), block->predecessors().end());
      for (auto const phi : block->phi_instructions()) {
        std::unordered_set<BasicBlock*> visited;
        for (auto const phi_input : phi->phi_inputs()) {
          auto const predecessor = phi_input->basic_block();
          if (!predecessors.count(predecessor)) {
            Error(ErrorCode::ValidatePhiInputInvalid, phi,
                  predecessor->value());
            continue;
          }
          if (visited.count(predecessor))
            Error(ErrorCode::ValidatePhiInputMultiple, phi,
                  predecessor->value());
          visited.insert(predecessor);
        }
        for (auto const predecessor : predecessors) {
          if (visited.count(predecessor))
            continue;
          Error(ErrorCode::ValidatePhiInputMissing, phi, predecessor->value());
        }
      }
    }

    Validate(block);

    if (block->last_instruction()->is<ExitInstruction>()) {
      if (found_exit)
        Error(ErrorCode::ValidateBasicBlockExit, block->value());
      found_exit = true;
    }
  }
  if (!found_exit)
    Error(ErrorCode::ValidateBasicBlockExit, function->value());
  return editor()->errors().empty();
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
  // All block operands must be alive.
  auto position = 0;
  for (auto const target_block : instruction->block_operands()) {
    if (!target_block->id())
      Error(ErrorCode::ValidateInstructionBlockOperand, instruction, position);
    ++position;
  }
  // Instruction specific validation.
  instruction->Accept(this);
  return editor()->errors().empty();
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

void Validator::VisitExtend(ExtendInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (output.type != Value::Type::Float)
    Error(ErrorCode::ValidateInstructionOutputType, instr, 0);
  if (input.type != Value::Type::Float)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::ByteSizeOf(output) <= Value::ByteSizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

void Validator::VisitRet(RetInstruction* instr) {
  if (instr->block_operand(0) != exit_block())
    Error(ErrorCode::ValidateInstructionBlockOperand, instr, 0);
}

void Validator::VisitSignedConvert(SignedConvertInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (output.type == input.type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
}

void Validator::VisitSignExtend(SignExtendInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (output.type != Value::Type::Integer)
    Error(ErrorCode::ValidateInstructionOutputType, instr, 0);
  if (input.type != Value::Type::Integer)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::ByteSizeOf(output) <= Value::ByteSizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

void Validator::VisitTruncate(TruncateInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (output.type != input.type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::ByteSizeOf(output) >= Value::ByteSizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

void Validator::VisitUnsignedConvert(UnsignedConvertInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (output.type == input.type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
}

void Validator::VisitZeroExtend(ZeroExtendInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (output.type != Value::Type::Integer)
    Error(ErrorCode::ValidateInstructionOutputType, instr, 0);
  if (input.type != Value::Type::Integer)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::ByteSizeOf(output) <= Value::ByteSizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

}  // namespace lir
}  // namespace elang
