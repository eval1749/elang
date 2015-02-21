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

}  // namespace

Value StackAssigner::MapToStackSlot(Value proxy) {
  DCHECK_NE(return_address_offset_, -1);
  auto const kAlignment = 8;
  if (proxy.is_parameter()) {
    return Value::StackSlot(
        proxy, return_address_offset_ + kAlignment * (proxy.data + 1));
  }
  if (proxy.is_spill_slot())
    return Value::StackSlot(proxy, proxy.data);
  NOTREACHED() << proxy << " isn't memory proxy.";
  return Value();
}

// Stack layout of leaf function; not use RBP for accessing local variable.
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
// RSP+32   | param[0]       | RCX home
//          +----------------+
// RSP+48   | param[1]       | RDX home
//          +----------------+
// RSP+56   | param[2]       | R8 home
//          +----------------+
// RSP+64   | param[3]       | R9 home
//          +----------------+
// RSP+72   | param[4]       |
//          +----------------+
//
void StackAssigner::RunForLeafFunction() {
  DCHECK_EQ(return_address_offset_, -1);

  auto const kAlignment = 8;
  auto const size =
      stack_assignments_->maximum_size() +
      static_cast<int>(stack_assignments_->preserving_registers().size()) *
          kAlignment;
  if (size) {
    // Allocate slots for local variable on stack.
    auto const rsp = Target::GetRegister(isa::RSP);
    auto const size64 = Value::SmallInt64(size);
    stack_assignments_->prologue_instructions_.push_back(
        factory()->NewSubInstruction(rsp, rsp, size64));
  }

  return_address_offset_ = size;

  // Allocate spill slot to stack
  for (auto pair : register_assignments_.proxy_map()) {
    auto const proxy = pair.second;
    SetStackSlot(proxy, MapToStackSlot(proxy));
  }

  // Save/restore preserving registers
  for (auto const pair : stack_assignments_->preserving_registers()) {
    auto const physical = pair.first;
    auto const slot_proxy = pair.second;
    auto const stack_slot = MapToStackSlot(slot_proxy);
    stack_assignments_->prologue_instructions_.push_back(
        factory()->NewCopyInstruction(stack_slot, physical));
    stack_assignments_->epilogue_instructions_.push_back(
        factory()->NewCopyInstruction(physical, stack_slot));
  }

  if (!size)
    return;

  // Deallocate slots for local variable on stack.
  auto const rsp = Target::GetRegister(isa::RSP);
  auto const size64 = Value::SmallInt64(size);
  stack_assignments_->epilogue_instructions_.push_back(
      factory()->NewAddInstruction(rsp, rsp, size64));
}

void StackAssigner::RunForNonLeafFunction() {
  NOTREACHED() << "NYI: StackAssigner::RunForNonLeafFunction()";
}

}  // namespace lir
}  // namespace elang
