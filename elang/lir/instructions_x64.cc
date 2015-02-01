// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/instructions.h"
#include "elang/lir/isa_x64.h"

namespace elang {
namespace lir {

EntryInstruction::EntryInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::nop, 0, 0) {
  DCHECK(factory);
}

CallInstruction::CallInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::call_Jv, 0, 1) {
  DCHECK(factory);
}

ExitInstruction::ExitInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::nop, 0, 0) {
  DCHECK(factory);
}

JumpInstruction::JumpInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::jmp_Jv, 0, 1) {
  DCHECK(factory);
}

RetInstruction::RetInstruction(Factory* factory)
    : Instruction(factory, isa::Opcode::ret, 0, 0) {
  DCHECK(factory);
}

}  // namespace lir
}  // namespace elang
