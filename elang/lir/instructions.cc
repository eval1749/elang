// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

#define V(Name, ...)                                            \
  void Name##Instruction::Accept(InstructionVisitor* visitor) { \
    visitor->Visit##Name(this);                                 \
  }
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
Instruction::Instruction(Factory* factory,
                         isa::Opcode opcode,
                         int output_count,
                         int input_count)
    : basic_block_(nullptr),
      id_(0),
      opcode_(opcode),
      inputs_(factory->zone(), input_count),
      outputs_(factory->zone(), output_count) {
}

bool Instruction::IsTerminator() const {
  return false;
}

bool ExitInstruction::IsTerminator() const {
  return true;
}

bool ReturnInstruction::IsTerminator() const {
  return true;
}

}  // namespace lir
}  // namespace elang
