// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/instructions.h"
#include "elang/lir/isa_x64.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

EntryInstruction::EntryInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::nop, 0, 0) {
}

CallInstruction::CallInstruction(Factory* factory, Value callee)
    : Instruction(factory, isa::Opcode::call_Jv, 0, 1) {
  InitInput(0, callee);
}

ExitInstruction::ExitInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::nop, 0, 0) {
}

JumpInstruction::JumpInstruction(Factory* factory, BasicBlock* target_block)
    : Instruction(factory, isa::Opcode::jmp_Jv, 0, 1) {
  InitInput(0, target_block->value());
}

LoadInstruction::LoadInstruction(Factory* factory, Value output, Value input)
    : Instruction(factory, isa::Opcode::mov_Gv_Ev, 1, 1) {
  DCHECK(!input.is_register());
  DCHECK(output.is_register());
  InitOutput(0, output);
  InitInput(0, input);
}

RetInstruction::RetInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::ret, 0, 0) {
}

}  // namespace lir
}  // namespace elang
