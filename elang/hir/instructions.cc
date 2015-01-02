// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
bool Instruction::CanBeRemoved() const {
  return !IsTerminator() && users().empty();
}

bool Instruction::IsExit() const {
  return false;
}

bool Instruction::IsTerminator() const {
  return false;
}

Instruction::Instruction(Type* output_type) : Operand(output_type) {
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
CallInstruction::CallInstruction(Factory* factory,
                                 Type* output_type,
                                 Operand* callee,
                                 Operand* arguments)
    : InstructionTemplate(output_type, callee, arguments) {
  __assume(factory);
}

bool CallInstruction::CanBeRemoved() const {
  // TODO(eval1749) We should return true for known side effect free functions.
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
ReturnInstruction::ReturnInstruction(Factory* factory,
                                     Type* output_type,
                                     Operand* value)
    : InstructionTemplate(output_type, value) {
}

ReturnInstruction* ReturnInstruction::New(Factory* factory, Operand* value) {
  return InstructionTemplate::New(factory, factory->types()->GetVoidType(),
                                  value);
}

bool ReturnInstruction::IsExit() const {
  return true;
}

bool ReturnInstruction::IsTerminator() const {
  return true;
}

}  // namespace hir
}  // namespace elang
