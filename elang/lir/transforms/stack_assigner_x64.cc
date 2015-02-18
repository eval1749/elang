// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_assigner.h"

#include "elang/lir/transforms/stack_assignments.h"

namespace elang {
namespace lir {

namespace {
int RoundUp(int value, int alignment) {
  return (value + alignment - 1) / alignment * alignment;
}
}  // namespace

void StackAssigner::Run() {
  if (editor()->IsLeafFunction()) {
    RunForLeafFunction();
    return;
  }
  RunForNonLeafFunction();
}

//
//          +----------------+
// RSP+64   | R9 home        |
//          +----------------+
// RSP+56   | R8 home        |
//          +----------------+
// RSP+48   | RDX home       |
//          +----------------+
// RSP+32   | RCX home       |
//          +----------------+
// RSP+24   | return address |
//          +----------------+
//          | local[16]      |
//          +----------------+
//          | local[8]       |
//          +----------------+
// RSP-->   | local[0]       |
//          +----------------+

void StackAssigner::RunForLeafFunction() {
  auto const using_size = assignments_->maximum_size() +
                          assignments_->preserving_registers().size() * 8;
  auto const pre_allocated_size = editor()->function()->number_of_parameters();
  auto const size = pre_allocated_size > using_size
      ? 0 : RoundUp(using_size - pre_allocated_size, 8);
  if (size) {
    assignments_->prologue_instructions_.push_back(
        factory()->NewSubInstruction(
            Traget::GetRegister(isa::RSP),
            Traget::GetRegister(isa::RSP),
            Value::Immediate(ValueSize::Size64, size)));
    assignments_->epilogue_instructions_.push_back(
        factory()->NewAddInstruction(
            Traget::GetRegister(isa::RSP),
            Traget::GetRegister(isa::RSP),
            Value::Immediate(ValueSize::Size64, size)));
  }
  auto offset = editor()->function()->number_of_parameters() + size + 8;
  auto return_offset = size;
  for (auto const physical : assignments_->preserving_registers()) {
    if (offset == return_offset)
      offset -= 8;
    assignments_->prologue_instructions_.push_back(
        factory()->NewCopyInstruction(Value::StackSlot(offset), physical));
    offset -= 8;
  }
  for (auto& pair : register_assignments_->stack_slot_map_) {
    auto slot_offset = pair.data + offset;
    if (slot_offset >= return_offset)
      slot_offset += 8;
    pair.second = Value::StackSlot(slot_offset);
  }
}

}  // namespace lir
}  // namespace elang
