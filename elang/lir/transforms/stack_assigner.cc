// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_assigner.h"

#include "base/logging.h"
#include "elang/lir/transforms/stack_assignments.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// StackAssigner
//
StackAssigner::StackAssigner(Factory* factory,
                             RegisterAssignments* register_assignments,
                             StackAssignments* stack_assignments)
    : FactoryUser(factory),
      register_assignments_(register_assignments),
      stack_assignments_(stack_assignments) {
}

StackAssigner::~StackAssigner() {
}

void StackAssigner::AddEpilogue(Instruction* instruction) {
  stack_assignments_->epilogue_instructions_.push_back(instruction);
}

void StackAssigner::AddPrologue(Instruction* instruction) {
  stack_assignments_->prologue_instructions_.push_back(instruction);
}

void StackAssigner::Run() {
  if (!stack_assignments_->number_of_calls()) {
    RunForLeafFunction();
    return;
  }
  RunForNonLeafFunction();
}

void StackAssigner::SetStackSlot(Value proxy, Value stack_slot) {
  DCHECK(proxy.is_memory_proxy());
  DCHECK(stack_slot.is_stack_slot() || stack_slot.is_frame_slot());
  DCHECK_EQ(proxy.type, stack_slot.type);
  DCHECK_EQ(proxy.size, stack_slot.size);
  DCHECK(!stack_assignments_->stack_map_.count(proxy));
  stack_assignments_->stack_map_[proxy] = stack_slot;
}

}  // namespace lir
}  // namespace elang
