// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_assigner.h"

#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/transforms/stack_assignments.h"

namespace elang {
namespace lir {

namespace {

#if 0
// TODO(eval1749) Move |IsLeafFunction()| somewhere to share code.
bool IsLeafFunction(const Function* function) {
  for (auto const block : function->basic_blocks()) {
    for (auto const instr : block->instructions()) {
      if (instr->is<CallInstruction>())
        return false;
    }
  }
  return true;
}
#endif

int RoundUp(int value, int alignment) {
  return (value + alignment - 1) / alignment * alignment;
}

int StackOffset(int offset, int return_offset) {
  return offset >= return_offset ? offset + 8 : offset;
}

}  // namespace

void StackAssigner::Run() {
  if (!stack_assignments_->number_of_calls()) {
    RunForLeafFunction();
    return;
  }
  RunForNonLeafFunction();
}

// Stack layout of leaf function; not use RBP.
//
//          +----------------+
// RSP ---->| local[0]       |
//          +----------------+
//          | local[8]       |
//          +----------------+
//          | local[16]      |
//          +----------------+
// RSP+24   | return address |
//          +----------------+
// RSP+32   | RCX home       |
//          +----------------+
// RSP+48   | RDX home       |
//          +----------------+
// RSP+56   | R8 home        |
//          +----------------+
// RSP+64   | R9 home        |
//          +----------------+
// RSP+72   | arg[4]         |
//          +----------------+
//
void StackAssigner::RunForLeafFunction() {
  auto const kAlignment = 8;
  auto const using_size =
      stack_assignments_->maximum_size() +
      static_cast<int>(stack_assignments_->preserving_registers().size()) *
          kAlignment;
  auto const pre_allocated_size = stack_assignments_->number_of_parameters();
  auto const size = pre_allocated_size > using_size
                        ? 0
                        : RoundUp(using_size - pre_allocated_size, kAlignment);
  if (size) {
    stack_assignments_->prologue_instructions_.push_back(
        factory()->NewSubInstruction(
            Target::GetRegister(isa::RSP), Target::GetRegister(isa::RSP),
            Value::Immediate(ValueSize::Size64, size)));
    stack_assignments_->epilogue_instructions_.push_back(
        factory()->NewAddInstruction(
            Target::GetRegister(isa::RSP), Target::GetRegister(isa::RSP),
            Value::Immediate(ValueSize::Size64, size)));
  }

  auto const return_offset = size;

  // Allocate spill slot to stack
  for (auto pair : register_assignments_.spill_slot_map()) {
    auto const spill_slot = pair.second;
    SetStackSlot(spill_slot,
                 Value::StackSlot(spill_slot,
                                  StackOffset(spill_slot.data, return_offset)));
  }

  // Restore preserving registers
  auto offset = StackOffset(using_size, return_offset);
  for (auto const physical : stack_assignments_->preserving_registers()) {
    auto const slot_offset = StackOffset(offset, return_offset);
    auto const stack_slot =
        Value::StackSlot(physical, StackOffset(slot_offset, return_offset));
    stack_assignments_->prologue_instructions_.push_back(
        factory()->NewCopyInstruction(stack_slot, physical));
    offset += kAlignment;
  }
}

void StackAssigner::RunForNonLeafFunction() {
  NOTREACHED() << "NYI: StackAssigner::RunForNonLeafFunction()";
}

}  // namespace lir
}  // namespace elang
