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
#include "elang/lir/target.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Validator
//
Validator::Validator(Editor* editor)
    : ErrorReporter(editor->factory()), editor_(editor) {
}

Validator::~Validator() {
}

BasicBlock* Validator::entry_block() const {
  return function()->entry_block();
}

BasicBlock* Validator::exit_block() const {
  return function()->exit_block();
}

Factory* Validator::factory() const {
  return editor()->factory();
}

Function* Validator::function() const {
  return editor()->function();
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

  // Check phi instructions
  for (auto const instruction : block->phi_instructions())
    Validate(instruction);

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
  return factory()->errors().empty();
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
  return factory()->errors().empty();
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
  return factory()->errors().empty();
}

void Validator::ValidateArithmeticInstruction(Instruction* instr) {
  DCHECK_EQ(1, instr->CountOutputs());
  DCHECK_EQ(2, instr->CountInputs());
  auto const output = instr->output(0);
  auto const input0 = instr->input(0);
  auto const input1 = instr->input(1);
  if (output.type != input0.type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (output.size != input0.size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
  if (output.type != input1.type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 1);
  if (output.size != input1.size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 1);
}

// InstructionVisitor

void Validator::VisitAdd(AddInstruction* instr) {
  ValidateArithmeticInstruction(instr);
}

void Validator::VisitBitAnd(BitAndInstruction* instr) {
  ValidateArithmeticInstruction(instr);
}

void Validator::VisitBitOr(BitOrInstruction* instr) {
  ValidateArithmeticInstruction(instr);
}

void Validator::VisitBitXor(BitXorInstruction* instr) {
  ValidateArithmeticInstruction(instr);
}

void Validator::VisitBranch(BranchInstruction* instr) {
  if (!instr->input(0).is_conditional())
    Error(ErrorCode::ValidateInstructionInput, instr, 0);
}

void Validator::VisitCmp(CmpInstruction* instr) {
  auto const output = instr->output(0);
  auto const left = instr->input(0);
  auto const right = instr->input(1);
  if (!output.is_conditional())
    Error(ErrorCode::ValidateInstructionOutput, instr, 0);
  if (!left.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (!right.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 1);
  if (left.size != right.size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 1);
}

void Validator::VisitCopy(CopyInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (!output.is_output())
    Error(ErrorCode::ValidateInstructionOutput, instr, 0);
  if (!input.is_output())
    Error(ErrorCode::ValidateInstructionInput, instr, 0);
  if (output.size != input.size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
  if (output.type != input.type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
}

void Validator::VisitDiv(DivInstruction* instr) {
  ValidateArithmeticInstruction(instr);
}

void Validator::VisitExtend(ExtendInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (!output.is_float())
    Error(ErrorCode::ValidateInstructionOutputType, instr, 0);
  if (!input.is_float())
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::SizeOf(output) <= Value::SizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

void Validator::VisitFloatCmp(FloatCmpInstruction* instr) {
  auto const output = instr->output(0);
  auto const left = instr->input(0);
  auto const right = instr->input(1);
  if (!output.is_conditional())
    Error(ErrorCode::ValidateInstructionOutput, instr, 0);
  if (!left.is_float())
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (!right.is_float())
    Error(ErrorCode::ValidateInstructionInputType, instr, 1);
  if (left.size != right.size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 1);
}

void Validator::VisitLoad(LoadInstruction* instr) {
  auto const array = instr->input(0);
  auto const pointer = instr->input(1);
  auto const offset = instr->input(2);
  if (!array.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (array.size != Target::IntPtrType().size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
  if (!pointer.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 1);
  if (pointer.size != Target::IntPtrType().size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 1);
  if (!offset.is_int32())
    Error(ErrorCode::ValidateInstructionInputType, instr, 2);
}

void Validator::VisitMul(MulInstruction* instr) {
  ValidateArithmeticInstruction(instr);
}

void Validator::VisitPhi(PhiInstruction* instr) {
  auto const output = instr->output(0);
  if (!output.is_virtual()) {
    // Output of 'phi' instruction must be virtual register.
    Error(ErrorCode::ValidateInstructionOutput, instr, 0);
  }
  auto position = 0;
  for (auto const phi_input : instr->phi_inputs()) {
    auto const input = phi_input->value();
    if (input.is_physical())
      Error(ErrorCode::ValidateInstructionInput, instr, position);
    if (input.size != output.size)
      Error(ErrorCode::ValidateInstructionInputSize, instr, position);
    if (input.type != output.type)
      Error(ErrorCode::ValidateInstructionInputType, instr, position);
    ++position;
  }
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
  if (!output.is_integer())
    Error(ErrorCode::ValidateInstructionOutputType, instr, 0);
  if (!input.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::SizeOf(output) <= Value::SizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

void Validator::VisitStore(StoreInstruction* instr) {
  // Note: We can't verify |input(3)| since we don't have information about
  // destination.
  auto const array = instr->input(0);
  auto const pointer = instr->input(1);
  auto const offset = instr->input(2);
  if (!array.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (array.size != Target::IntPtrType().size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
  if (!pointer.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 1);
  if (pointer.size != Target::IntPtrType().size)
    Error(ErrorCode::ValidateInstructionInputSize, instr, 1);
  if (!offset.is_int32())
    Error(ErrorCode::ValidateInstructionInputType, instr, 2);
}

void Validator::VisitSub(SubInstruction* instr) {
  ValidateArithmeticInstruction(instr);
}

void Validator::VisitTruncate(TruncateInstruction* instr) {
  auto const output = instr->output(0);
  auto const input = instr->input(0);
  if (output.type != input.type)
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::SizeOf(output) >= Value::SizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

void Validator::VisitUse(UseInstruction* instr) {
  if (!instr->input(0).is_output())
    Error(ErrorCode::ValidateInstructionInput, instr, 0);
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
  if (!output.is_integer())
    Error(ErrorCode::ValidateInstructionOutputType, instr, 0);
  if (!input.is_integer())
    Error(ErrorCode::ValidateInstructionInputType, instr, 0);
  if (Value::SizeOf(output) <= Value::SizeOf(input))
    Error(ErrorCode::ValidateInstructionInputSize, instr, 0);
}

}  // namespace lir
}  // namespace elang
