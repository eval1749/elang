// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/instructions.h"
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

// Load
LoadInstruction::LoadInstruction(Value output, Value input)
    : InstructionTemplate() {
  DCHECK(!input.is_register());
  DCHECK(output.is_register());
  InitOutput(0, output);
  InitInput(0, input);
}

base::StringPiece LoadInstruction::mnemonic() const {
  return "mov";
}

// Ret
RetInstruction::RetInstruction() {
}

base::StringPiece RetInstruction::mnemonic() const {
  return "ret";
}

}  // namespace lir
}  // namespace elang
