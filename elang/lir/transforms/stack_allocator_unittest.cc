// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <vector>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/value.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {
namespace {

//////////////////////////////////////////////////////////////////////
//
// LirStackAllocatorTest
//
class LirStackAllocatorTest : public testing::LirTest {
 protected:
  LirStackAllocatorTest() = default;
  ~LirStackAllocatorTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirStackAllocatorTest);
};

// Test cases...
TEST_F(LirStackAllocatorTest, Alignment) {
  std::array<Value, 5> vregs{
        factory()->NewRegister(Value::Int8Type()),
        factory()->NewRegister(Value::Int16Type()),
        factory()->NewRegister(Value::Int32Type()),
        factory()->NewRegister(Value::Int64Type()),
        factory()->NewRegister(Value::Int64Type()),
  };
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);

  StackAssignments assignments;
  StackAllocator allocator(&editor, &assignments);

  EXPECT_EQ(Value::SpillSlot(vregs[0], 0), allocator.Allocate(vregs[0]));
  EXPECT_EQ(Value::SpillSlot(vregs[1], 2), allocator.Allocate(vregs[1]));
  EXPECT_EQ(Value::SpillSlot(vregs[2], 4), allocator.Allocate(vregs[2]));
  EXPECT_EQ(Value::SpillSlot(vregs[3], 8), allocator.Allocate(vregs[3]));
  EXPECT_EQ(Value::SpillSlot(vregs[4], 16), allocator.Allocate(vregs[4]));
  EXPECT_EQ(24u, assignments.maximum_size());
}

TEST_F(LirStackAllocatorTest, Reuse) {
  std::array<Value, 2> vregs{
      factory()->NewRegister(Value::Int32Type()),
      factory()->NewRegister(Value::Int32Type()),
  };

  std::vector<Value> parameters{Target::GetParameterAt(vregs[0], 0)};
  auto const function = CreateFunctionEmptySample(parameters);
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(
      factory()->NewCopyInstruction(vregs[0], function->parameters()[0]));
  editor.Append(
      factory()->NewAddInstruction(vregs[1], vregs[0], Value::SmallInt32(42)));
  ASSERT_EQ("", Commit(&editor));

  StackAssignments assignments;
  StackAllocator allocator(&editor, &assignments);

  EXPECT_EQ(Value::SpillSlot(vregs[0], 0), allocator.Allocate(vregs[0]));
  allocator.Free(vregs[0]);
  EXPECT_EQ(Value::SpillSlot(vregs[1], 0), allocator.Allocate(vregs[1]));
  EXPECT_EQ(8u, assignments.maximum_size());
}

TEST_F(LirStackAllocatorTest, TrackNumberOfArguments) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);

  StackAssignments assignments;
  StackAllocator allocator(&editor, &assignments);

  allocator.TrackNumberOfArguments(4);
  allocator.TrackNumberOfArguments(3);
  allocator.TrackNumberOfArguments(2);
  allocator.TrackNumberOfArguments(0);
  EXPECT_EQ(4, assignments.maximum_argc());
}

}  // namespace
}  // namespace lir
}  // namespace elang
