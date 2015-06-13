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
  EXPECT_EQ(24u, assignments.maximum_variables_size());
}

TEST_F(LirStackAllocatorTest, Parameters) {
  std::array<Value, 10> vregs;
  std::vector<Value> parameters(vregs.size());
  for (auto position = 0; position < vregs.size(); ++position) {
    vregs[position] = factory()->NewRegister(Value::Int32Type());
    parameters[position] = Target::ParameterAt(vregs[position], position);
  }

  auto const function = CreateFunctionEmptySample(parameters);
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewCopyInstruction(vregs[0], parameters[9]));
  ASSERT_EQ("", Commit(&editor));

  StackAssignments assignments;
  StackAllocator allocator(&editor, &assignments);
  allocator.Assign(vregs[0], parameters[9]);

  EXPECT_EQ(parameters[9], allocator.AllocationFor(vregs[0]));
  EXPECT_EQ(0u, assignments.maximum_variables_size());
}

TEST_F(LirStackAllocatorTest, Reuse) {
  std::array<Value, 2> vregs{
      factory()->NewRegister(Value::Int32Type()),
      factory()->NewRegister(Value::Int32Type()),
  };

  std::vector<Value> parameters{Target::ParameterAt(vregs[0], 0)};
  auto const function = CreateFunctionEmptySample(parameters);
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewCopyInstruction(vregs[0], parameters[0]));
  editor.Append(factory()->NewIntAddInstruction(vregs[1], vregs[0],
                                                Value::SmallInt32(42)));
  ASSERT_EQ("", Commit(&editor));

  StackAssignments assignments;
  StackAllocator allocator(&editor, &assignments);

  EXPECT_EQ(Value::SpillSlot(vregs[0], 0), allocator.Allocate(vregs[0]));
  allocator.Free(vregs[0]);
  EXPECT_EQ(Value::SpillSlot(vregs[1], 0), allocator.Allocate(vregs[1]));
  EXPECT_EQ(8u, assignments.maximum_variables_size());
}

TEST_F(LirStackAllocatorTest, TrackCall) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  std::vector<Value> args{
      Target::ArgumentAt(Value::Int64Type(), 0),
      Target::ArgumentAt(Value::Int32Type(), 1),
      Target::ArgumentAt(Value::Int32Type(), 2),
      Target::ArgumentAt(Value::Int64Type(), 3),
      Target::ArgumentAt(Value::Int64Type(), 4),
  };
  std::vector<Value> values;
  for (auto const arg : args)
    values.push_back(NewIntValue(arg, static_cast<int>(values.size())));
  editor.Append(NewPCopyInstruction(args, values));
  auto const call_instr = NewCallInstruction({}, NewStringValue8("foo"));
  editor.Append(call_instr);
  ASSERT_EQ("", Commit(&editor));

  StackAssignments assignments;
  StackAllocator allocator(&editor, &assignments);
  allocator.TrackCall(call_instr);
  EXPECT_EQ(args.size() * 8, assignments.maximum_arguments_size());
}

}  // namespace
}  // namespace lir
}  // namespace elang
