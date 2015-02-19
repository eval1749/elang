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

void StackAssigner::SetStackSlot(Value spill_slot, Value stack_slot) {
  DCHECK(spill_slot.is_spill_slot());
  DCHECK(stack_slot.is_stack_slot());
  DCHECK_EQ(spill_slot.type, stack_slot.type);
  DCHECK_EQ(spill_slot.size, stack_slot.size);
  DCHECK(!stack_assignments_->stack_map_.count(spill_slot));
  stack_assignments_->stack_map_[spill_slot] = stack_slot;
}

}  // namespace lir
}  // namespace elang
