// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

// Constructors are implemented in "instructions_${arch}.cc".

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
Instruction::Instruction(Factory* factory, int output_count, int input_count)
    : basic_block_(nullptr),
      id_(0),
      inputs_(factory->zone(), input_count),
      outputs_(factory->zone(), output_count) {
}

void Instruction::InitInput(int index, Value new_input) {
  inputs_[index] = new_input;
}

void Instruction::InitOutput(int index, Value new_output) {
  DCHECK(new_output.is_register());
  outputs_[index] = new_output;
}

bool Instruction::IsTerminator() const {
  return false;
}

bool ExitInstruction::IsTerminator() const {
  return true;
}

bool JumpInstruction::IsTerminator() const {
  return true;
}

bool RetInstruction::IsTerminator() const {
  return true;
}

}  // namespace lir
}  // namespace elang
