// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/instructions_x64.h"
#include "elang/lir/isa_x64.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

// Branch
base::StringPiece BranchInstruction::mnemonic() const {
  return "br";
}

// Call
base::StringPiece CallInstruction::mnemonic() const {
  return "call";
}

// Copy
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
base::StringPiece EntryInstruction::mnemonic() const {
  return "entry";
}

// Exit
base::StringPiece ExitInstruction::mnemonic() const {
  return "exit";
}

// Jump
base::StringPiece JumpInstruction::mnemonic() const {
  return "jmp";
}

// Literal
base::StringPiece LiteralInstruction::mnemonic() const {
  return "mov";
}

// Load
base::StringPiece LoadInstruction::mnemonic() const {
  return "load";
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
base::StringPiece RetInstruction::mnemonic() const {
  return "ret";
}

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

base::StringPiece ModInstruction::mnemonic() const {
  return "mod";
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
