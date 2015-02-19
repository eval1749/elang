// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/value.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {
namespace {

// Test cases...
TEST(StackAllocatorTest, Alignment4) {
  auto const int16_type = Value(Value::Type::Integer, ValueSize::Size16);
  auto const int32_type = Value(Value::Type::Integer, ValueSize::Size32);
  auto const int64_type = Value(Value::Type::Integer, ValueSize::Size64);
  auto const int8_type = Value(Value::Type::Integer, ValueSize::Size8);

  StackAssignments assignments;
  StackAllocator allocator(&assignments, 4);

  EXPECT_EQ(Value::SpillSlot(int32_type, 0), allocator.Allocate(int32_type));
  EXPECT_EQ(Value::SpillSlot(int32_type, 4), allocator.Allocate(int32_type));
  EXPECT_EQ(Value::SpillSlot(int16_type, 8), allocator.Allocate(int16_type));
  EXPECT_EQ(Value::SpillSlot(int16_type, 10), allocator.Allocate(int16_type));
  EXPECT_EQ(12, allocator.RequiredSize()) << "stack is align 4 byte";

  EXPECT_EQ(Value::SpillSlot(int32_type, 12), allocator.Allocate(int32_type));
  ASSERT_EQ(16, allocator.RequiredSize()) << "stack is aligned 4 byte";

  EXPECT_EQ(Value::SpillSlot(int64_type, 16), allocator.Allocate(int64_type));
  ASSERT_EQ(24, allocator.RequiredSize()) << "stack is aligned 8 byte";

  EXPECT_EQ(Value::SpillSlot(int32_type, 24), allocator.Allocate(int32_type));
  auto const max_stack_size = 28;
  EXPECT_EQ(max_stack_size, allocator.RequiredSize())
      << "size of stack isn't changed";

  // Reuse free slots
  allocator.Free(Value::SpillSlot(int32_type, 0));
  EXPECT_EQ(Value::SpillSlot(int8_type, 0), allocator.Allocate(int8_type));
  EXPECT_EQ(Value::SpillSlot(int16_type, 2), allocator.Allocate(int16_type));
  EXPECT_EQ(Value::SpillSlot(int8_type, 1), allocator.Allocate(int8_type));
  allocator.Free(Value::SpillSlot(int16_type, 8));
  allocator.Free(Value::SpillSlot(int16_type, 10));
  EXPECT_EQ(Value::SpillSlot(int32_type, 8), allocator.Allocate(int32_type));

  EXPECT_EQ(max_stack_size, allocator.RequiredSize())
      << "size of stack isn't changed";
}

TEST(StackAllocatorTest, Alignment8) {
  auto const int16_type = Value(Value::Type::Integer, ValueSize::Size16);
  auto const int32_type = Value(Value::Type::Integer, ValueSize::Size32);
  auto const int64_type = Value(Value::Type::Integer, ValueSize::Size64);
  auto const int8_type = Value(Value::Type::Integer, ValueSize::Size8);

  StackAssignments assignments;
  StackAllocator allocator(&assignments, 8);

  EXPECT_EQ(Value::SpillSlot(int32_type, 0), allocator.Allocate(int32_type));
  EXPECT_EQ(Value::SpillSlot(int32_type, 4), allocator.Allocate(int32_type));
  EXPECT_EQ(Value::SpillSlot(int16_type, 8), allocator.Allocate(int16_type));
  EXPECT_EQ(Value::SpillSlot(int32_type, 12), allocator.Allocate(int32_type));
  EXPECT_EQ(16, allocator.RequiredSize());

  EXPECT_EQ(Value::SpillSlot(int32_type, 16), allocator.Allocate(int32_type));
  ASSERT_EQ(24, allocator.RequiredSize()) << "stack is aligned 8 byte";

  EXPECT_EQ(Value::SpillSlot(int64_type, 24), allocator.Allocate(int64_type));
  auto const max_stack_size = 32;
  ASSERT_EQ(max_stack_size, allocator.RequiredSize())
      << "stack is aligned 8 byte";

  EXPECT_EQ(Value::SpillSlot(int32_type, 20), allocator.Allocate(int32_type));
  EXPECT_EQ(max_stack_size, allocator.RequiredSize())
      << "size of stack isn't changed";

  // Reuse free slots
  allocator.Free(Value::SpillSlot(int32_type, 0));
  EXPECT_EQ(Value::SpillSlot(int8_type, 0), allocator.Allocate(int8_type));
  EXPECT_EQ(Value::SpillSlot(int16_type, 2), allocator.Allocate(int16_type));
  EXPECT_EQ(Value::SpillSlot(int8_type, 1), allocator.Allocate(int8_type));
  allocator.Free(Value::SpillSlot(int16_type, 8));
  EXPECT_EQ(Value::SpillSlot(int32_type, 8), allocator.Allocate(int32_type));

  EXPECT_EQ(max_stack_size, allocator.RequiredSize())
      << "size of stack isn't changed";
}

TEST(StackAllocatorTest, AllocateAt) {
  StackAssignments assignments;
  StackAllocator allocator(&assignments, 4);
  allocator.Allocate(Value::Int32Type());
  auto const spill_slot1 = allocator.Allocate(Value::Int32Type());
  allocator.Allocate(Value::Int32Type());

  allocator.Reset();
  allocator.AllocateAt(spill_slot1);
  EXPECT_EQ(Value::SpillSlot(Value::Int32Type(), 0),
            allocator.Allocate(Value::Int32Type()));

  EXPECT_EQ(12, assignments.maximum_size());
  EXPECT_EQ(12, allocator.RequiredSize());
}

TEST(StackAllocatorTest, TrackNumberOfArguments) {
  StackAssignments assignments;
  StackAllocator allocator(&assignments, 4);
  allocator.TrackNumberOfArguments(4);
  allocator.TrackNumberOfArguments(3);
  allocator.TrackNumberOfArguments(2);
  allocator.TrackNumberOfArguments(0);
  EXPECT_EQ(4, assignments.maximum_argc());
}

}  // namespace
}  // namespace lir
}  // namespace elang
