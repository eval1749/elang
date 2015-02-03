// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/instructions_x64.h"
#include "elang/lir/isa_x64.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

// Call
CallInstruction::CallInstruction(Value callee) {
  InitInput(0, callee);
}

base::StringPiece CallInstruction::mnemonic() const {
  return "call";
}

// Copy
CopyInstruction::CopyInstruction(Value output, Value input) {
  DCHECK(input.is_register());
  DCHECK(output.is_register());
  DCHECK_NE(output, input);
  DCHECK_EQ(output.size, input.size);
  DCHECK_EQ(output.type, input.type);
  InitOutput(0, output);
  InitInput(0, input);
}

base::StringPiece CopyInstruction::mnemonic() const {
  return "mov";
}

// DivX64
DivX64Instruction::DivX64Instruction(Value div_output,
                                     Value mod_output,
                                     Value high_left,
                                     Value low_left,
                                     Value right) {
  InitOutput(0, div_output);
  InitOutput(1, mod_output);
  InitInput(0, high_left);
  InitInput(1, low_left);
  InitInput(2, right);
}

base::StringPiece DivX64Instruction::mnemonic() const {
  return "div";
}

// Entry
EntryInstruction::EntryInstruction() {
}

base::StringPiece EntryInstruction::mnemonic() const {
  return "entry";
}

// Exit
ExitInstruction::ExitInstruction() : InstructionTemplate() {
}

base::StringPiece ExitInstruction::mnemonic() const {
  return "exit";
}

// Jump
JumpInstruction::JumpInstruction(BasicBlock* target_block) {
  InitInput(0, target_block->value());
}

base::StringPiece JumpInstruction::mnemonic() const {
  return "jmp";
}

// Literal
LiteralInstruction::LiteralInstruction(Value output, Value input)
    : InstructionTemplate() {
  DCHECK(input.is_immediate() || input.is_literal());
  DCHECK(output.is_register());
  DCHECK_EQ(output.size, input.size);
  DCHECK_EQ(output.type, input.type);
  InitOutput(0, output);
  InitInput(0, input);
}

base::StringPiece LiteralInstruction::mnemonic() const {
  return "mov";
}

// Load
LoadInstruction::LoadInstruction(Value output, Value input)
    : InstructionTemplate() {
  DCHECK(!input.is_immediate());
  DCHECK(!input.is_literal());
  DCHECK(!input.is_register());
  DCHECK(output.is_register());
  InitOutput(0, output);
  InitInput(0, input);
}

base::StringPiece LoadInstruction::mnemonic() const {
  return "mov";
}

// MulX64
MulX64Instruction::MulX64Instruction(Value high_output,
                                     Value low_output,
                                     Value left,
                                     Value right) {
  InitOutput(0, high_output);
  InitOutput(1, low_output);
  InitInput(0, left);
  InitInput(1, right);
}

base::StringPiece MulX64Instruction::mnemonic() const {
  return "mul";
}

// Ret
RetInstruction::RetInstruction() {
}

base::StringPiece RetInstruction::mnemonic() const {
  return "ret";
}

#define V(Name)                                                               \
  Name##Instruction::Name##Instruction(Value output, Value left, Value right) \
      : InstructionTemplate() {                                               \
    DCHECK(output.is_register());                                             \
    InitOutput(0, output);                                                    \
    InitInput(0, left);                                                       \
    InitInput(1, right);                                                      \
  }
FOR_EACH_LIR_INSTRUCTION_1_2(V)
#undef V

base::StringPiece AddInstruction::mnemonic() const {
  return "add";
}

base::StringPiece BitAndInstruction::mnemonic() const {
  return "and";
}

base::StringPiece BitOrInstruction::mnemonic() const {
  return "or";
}

base::StringPiece BitXorInstruction::mnemonic() const {
  return "xor";
}

base::StringPiece DivInstruction::mnemonic() const {
  return "div";
}

base::StringPiece EqInstruction::mnemonic() const {
  return "eq";
}

base::StringPiece GeInstruction::mnemonic() const {
  return "ge";
}

base::StringPiece GtInstruction::mnemonic() const {
  return "gt";
}

base::StringPiece LeInstruction::mnemonic() const {
  return "le";
}

base::StringPiece LtInstruction::mnemonic() const {
  return "lt";
}

base::StringPiece MulInstruction::mnemonic() const {
  return "mul";
}

base::StringPiece NeInstruction::mnemonic() const {
  return "ne";
}

base::StringPiece ShlInstruction::mnemonic() const {
  return "shl";
}

base::StringPiece ShrInstruction::mnemonic() const {
  return "shr";
}

base::StringPiece SubInstruction::mnemonic() const {
  return "sub";
}

}  // namespace lir
}  // namespace elang
