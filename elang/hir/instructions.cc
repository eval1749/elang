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
Instruction::Instruction(Type* output_type)
    : Value(output_type), basic_block_(nullptr), id_(0) {
}

bool Instruction::CanBeRemoved() const {
  return !IsTerminator() && users().empty();
}

bool Instruction::IsTerminator() const {
  return false;
}

#define V(Name, ...) \
  Instruction::Opcode Name##Instruction::opcode() const { return Opcode::Name; }
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

void Instruction::Accept(ValueVisitor* visitor) {
  DCHECK(visitor);
  NOTREACHED();
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
CallInstruction::CallInstruction(Type* output_type,
                                 Value* callee,
                                 Value* arguments)
    : InstructionTemplate(output_type, callee, arguments) {
}

bool CallInstruction::CanBeRemoved() const {
  // TODO(eval1749) We should return true for known side effect free functions.
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// EntryInstruction
//
EntryInstruction::EntryInstruction(Type* output_type)
    : InstructionTemplate(output_type) {
  DCHECK(output_type->is<VoidType>());
}

//////////////////////////////////////////////////////////////////////
//
// ExitInstruction
//
ExitInstruction::ExitInstruction(Type* output_type)
    : InstructionTemplate(output_type) {
  DCHECK(output_type->is<VoidType>());
}

bool ExitInstruction::IsTerminator() const {
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
ReturnInstruction::ReturnInstruction(Type* output_type,
                                     Value* value,
                                     BasicBlock* exit_block)
    : InstructionTemplate(output_type, value, exit_block) {
  DCHECK(output_type->is<VoidType>());
  DCHECK(!value->is<BasicBlock>());
  DCHECK(exit_block->last_instruction()->is<ExitInstruction>());
}

bool ReturnInstruction::IsTerminator() const {
  return true;
}

}  // namespace hir
}  // namespace elang
