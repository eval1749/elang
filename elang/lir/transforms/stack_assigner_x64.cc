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

auto const kAlignment = 8;

int RoundUp(int value, int alignment) {
  return (value + alignment - 1) / alignment * alignment;
}

}  // namespace

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
  auto const size = stack_assignments_->maximum_variables_size();

  if (size) {
    // Allocate slots for local variable on stack.
    auto const rsp = Target::GetRegister(isa::RSP);
    AddPrologue(NewSubInstruction(rsp, rsp, Value::SmallInt64(size)));
  }

  struct Mapper {
    int return_address_offset;

    explicit Mapper(int size) : return_address_offset(size) {}

    Value MapToStackSlot(Value proxy) {
      if (proxy.is_parameter()) {
        return Value::StackSlot(
            proxy, return_address_offset + kAlignment * (proxy.data + 1));
      }
      if (proxy.is_spill_slot())
        return Value::StackSlot(proxy, proxy.data);
      NOTREACHED() << proxy << " isn't memory proxy.";
      return Value();
    }
  } mapper(size);

  // Assign spill slot to stack
  for (auto const pair : register_assignments_.proxy_map()) {
    auto const proxy = pair.second;
    SetStackSlot(proxy, mapper.MapToStackSlot(proxy));
  }

  // Save/restore preserving registers
  for (auto const pair : stack_assignments_->preserving_registers()) {
    auto const physical = pair.first;
    auto const slot_proxy = pair.second;
    auto const stack_slot = mapper.MapToStackSlot(slot_proxy);
    AddPrologue(NewCopyInstruction(stack_slot, physical));
    AddEpilogue(NewCopyInstruction(physical, stack_slot));
  }

  if (!size)
    return;

  // Deallocate slots for local variable on stack.
  auto const rsp = Target::GetRegister(isa::RSP);
  auto const size64 = Value::SmallInt64(size);
  AddEpilogue(NewAddInstruction(rsp, rsp, size64));
}

// Stack layout of leaf function; not use RBP for accessing local variable.
//
//          +----------------+
// RSP ---->| arg[0]         | RCX home
//          +----------------+
//          | arg[1]         | RDX home
//          +----------------+
//          | arg[2]         | R8 home
//          +----------------+
//          | arg[3]         | R9 home
//          +----------------+
//          | arg[4]         |
//          +----------------+
//          | old RBP        |
//          +----------------+
// RSP+48   | local[0]       | <-- RBP
//          +----------------+
//          | local[8]       |
//          +----------------+
//          | local[16]      |
//          +----------------+
//          | padding        | padding to make RSP % 16 == 0
//          +----------------+
// RSP+72   | return address |
//          +----------------+
// RSP+80   | param[0]       | RCX home
//          +----------------+
// RSP+88   | param[1]       | RDX home
//          +----------------+
// RSP+96   | param[2]       | R8 home
//          +----------------+
// RSP+104  | param[3]       | R9 home
//          +----------------+
// RSP+112  | param[4]       |
//          +----------------+
//
void StackAssigner::RunForNonLeafFunction() {
  auto const args_size =
      stack_assignments_->maximum_arguments_size() * kAlignment;
  auto const local_size = stack_assignments_->maximum_variables_size();
  auto const using_size = args_size + local_size + (local_size ? 8 : 0);
  auto const size = using_size & 8 ? using_size : using_size + 8;
  auto const base_offset = local_size > 128 ? -128 : 0;

  if (size) {
    // Allocate slots for local variable on stack.
    auto const rbp = Target::GetRegister(isa::RBP);
    auto const rsp = Target::GetRegister(isa::RSP);
    AddPrologue(NewSubInstruction(rsp, rsp, Value::SmallInt64(size)));
    if (local_size) {
      AddPrologue(NewCopyInstruction(Value::StackSlot(rbp, args_size), rbp));
      // TODO(eval1749) We should use |lea rbp, [rsp+arg_size+base_offset]|
      AddPrologue(NewCopyInstruction(rbp, rsp));
      AddPrologue(NewAddInstruction(
          rbp, rbp, Value::SmallInt64(args_size + 8 + base_offset)));
    }
  }

  struct Mapper {
    int base_offset;
    int return_address_offset;

    Mapper(int size, int base_offset)
        : base_offset(base_offset), return_address_offset(size) {}

    Value MapToFrameSlot(Value proxy) {
      DCHECK_NE(return_address_offset, -1);
      if (proxy.is_argument())
        return Value::StackSlot(proxy, kAlignment * proxy.data);
      if (proxy.is_parameter()) {
        return Value::FrameSlot(
            proxy,
            return_address_offset + kAlignment * (proxy.data + base_offset));
      }
      if (proxy.is_spill_slot())
        return Value::FrameSlot(proxy, proxy.data);
      NOTREACHED() << proxy << " isn't memory proxy.";
      return Value();
    }
  } mapper(local_size, base_offset);

  // Assign spill slot to stack
  for (auto const pair : register_assignments_.proxy_map()) {
    auto const proxy = pair.second;
    SetStackSlot(proxy, mapper.MapToFrameSlot(proxy));
  }

  // Assign argument slot to stack
  for (auto const proxy : stack_assignments_->arguments())
    SetStackSlot(proxy, mapper.MapToFrameSlot(proxy));

  // Save/restore preserving registers
  for (auto const pair : stack_assignments_->preserving_registers()) {
    auto const physical = pair.first;
    auto const slot_proxy = pair.second;
    auto const stack_slot = mapper.MapToFrameSlot(slot_proxy);
    AddPrologue(NewCopyInstruction(stack_slot, physical));
    AddEpilogue(NewCopyInstruction(physical, stack_slot));
  }

  if (!size)
    return;

  // Deallocate slots for local variable on stack.
  auto const rbp = Target::GetRegister(isa::RBP);
  auto const rsp = Target::GetRegister(isa::RSP);
  auto const size64 = Value::SmallInt64(size);
  if (local_size)
    AddEpilogue(NewCopyInstruction(rbp, Value::StackSlot(rbp, args_size)));
  AddEpilogue(NewAddInstruction(rsp, rsp, size64));
}

}  // namespace lir
}  // namespace elang
