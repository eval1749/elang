// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/transforms/stack_assigner.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirStackAssignerX64Test
//
class LirStackAssignerX64Test : public testing::LirTest {
 protected:
  LirStackAssignerX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirStackAssignerX64Test);
};

// Test cases...

TEST_F(LirStackAssignerX64Test, Empty) {
  RegisterAssignments register_assignments;
  StackAssignments stack_assignments;
  StackAssigner stack_assigner(factory(), &register_assignments,
                               &stack_assignments);
  stack_assigner.Run();
}

TEST_F(LirStackAssignerX64Test, LeafFunction) {
  RegisterAssignments register_assignments;
  StackAssignments stack_assignments;

  RegisterAssignments::Editor editor(&register_assignments);
  StackAllocator stack_allocator(&stack_assignments, 8);

  std::array<Value, 5> vregs;
  for (auto& vreg : vregs) {
    vreg = factory()->NewRegister(ValueSize::Size64);
    auto const spill_slot = stack_allocator.Allocate(vreg);
    editor.SetSpillSlot(vreg, spill_slot);
  }
  StackAssigner stack_assigner(factory(), &register_assignments,
                               &stack_assignments);
  stack_assigner.Run();
  auto offset = 0;
  for (auto vreg : vregs) {
    auto const spill_slot = register_assignments.SpillSlotFor(vreg);
    EXPECT_EQ(Value::StackSlot(vreg, offset),
              stack_assignments.StackSlotOf(spill_slot))
        << " stack slot for " << vreg << " " << spill_slot;
    offset += Value::ByteSize(vreg.size);
  }
}

}  // namespace lir
}  // namespace elang
