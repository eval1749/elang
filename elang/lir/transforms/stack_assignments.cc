// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_assignments.h"

#include "base/logging.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// StackAssignments
//
StackAssignments::StackAssignments()
    : maximum_argc_(0),
      maximum_size_(0),
      number_of_calls_(0),
      number_of_parameters_(0),
      slot_count_(0) {
}

StackAssignments::~StackAssignments() {
}

Value StackAssignments::StackSlotOf(Value spill_slot) const {
  DCHECK(spill_slot.is_spill_slot());
  auto const it = stack_map_.find(spill_slot);
  DCHECK(it != stack_map_.end());
  return it->second;
}

}  // namespace lir
}  // namespace elang
