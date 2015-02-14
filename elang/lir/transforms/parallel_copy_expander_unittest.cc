// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/transforms/parallel_copy_expander.h"

namespace elang {
namespace lir {
namespace {

//////////////////////////////////////////////////////////////////////
//
// LirParallelCopyExpanderTest
//
class LirParallelCopyExpanderTest : public testing::LirTest {
 protected:
  LirParallelCopyExpanderTest() = default;
  ~LirParallelCopyExpanderTest() = default;

  static Value int32_type() {
    return Value(Value::Type::Integer, ValueSize::Size32);
  }

  static Value physical(int data) {
    return Value(Value::Type::Integer, ValueSize::Size32,
                 Value::Kind::PhysicalRegister, data);
  }

  static Value stack_slot(int data) {
    return Value(Value::Type::Integer, ValueSize::Size32,
                 Value::Kind::StackSlot, data);
  }

  std::string Expand(ParallelCopyExpander* expander);

 private:
  DISALLOW_COPY_AND_ASSIGN(LirParallelCopyExpanderTest);
};

std::string LirParallelCopyExpanderTest::Expand(
    ParallelCopyExpander* expander) {
  std::stringstream ostream;
  for (auto const instr : expander->Expand())
    ostream << PrintAsGeneric(instr) << std::endl;
  return ostream.str();
}

// Test cases...

TEST_F(LirParallelCopyExpanderTest, Basic) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(physical(0), physical(1));
  expander.AddTask(physical(2), physical(1));
  expander.AddTask(physical(4), physical(3));
  EXPECT_EQ(
      "mov R0 = R1\n"
      "mov R2 = R1\n"
      "mov R4 = R3\n",
      Expand(&expander));
}

TEST_F(LirParallelCopyExpanderTest, MemorySwap) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(stack_slot(0), stack_slot(1));
  expander.AddTask(stack_slot(1), stack_slot(0));
  expander.AddScratch(physical(2));
  expander.AddScratch(physical(3));
  EXPECT_EQ(
      "mov R3 = sp[0]\n"
      "mov R2 = sp[1]\n"
      "mov sp[1] = R3\n"
      "mov sp[0] = R2\n",
      Expand(&expander));
}

TEST_F(LirParallelCopyExpanderTest, MemorySwapNoScratch) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(stack_slot(0), stack_slot(1));
  expander.AddTask(stack_slot(1), stack_slot(0));
  EXPECT_EQ("", Expand(&expander)) << "memory swap requires 2 scratch register";
}

TEST_F(LirParallelCopyExpanderTest, MemorySwapOneScratch) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(stack_slot(0), stack_slot(1));
  expander.AddTask(stack_slot(1), stack_slot(0));
  EXPECT_EQ("", Expand(&expander)) << "memory swap requires 2 scratch register";
}

TEST_F(LirParallelCopyExpanderTest, PhysicalToMemory) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(stack_slot(0), physical(0));
  expander.AddTask(stack_slot(1), physical(1));
  EXPECT_EQ(
      "mov sp[0] = R0\n"
      "mov sp[1] = R1\n",
      Expand(&expander));
}

TEST_F(LirParallelCopyExpanderTest, Rotate) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(physical(0), physical(1));
  expander.AddTask(physical(1), physical(2));
  expander.AddTask(physical(2), physical(0));
  EXPECT_EQ(
      "pcopy R2, R0 = R0, R2\n"
      "pcopy R1, R0 = R0, R1\n",
      Expand(&expander));
}

TEST_F(LirParallelCopyExpanderTest, RotateMemory) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(stack_slot(0), stack_slot(1));
  expander.AddTask(stack_slot(1), stack_slot(2));
  expander.AddTask(stack_slot(2), stack_slot(0));
  expander.AddScratch(physical(4));
  expander.AddScratch(physical(5));
  EXPECT_EQ(
      "mov R5 = sp[0]\n"
      "mov R4 = sp[2]\n"
      "mov sp[2] = R5\n"
      "mov R5 = sp[1]\n"
      "mov sp[0] = R5\n"
      "mov sp[1] = R4\n",
      Expand(&expander));
}

TEST_F(LirParallelCopyExpanderTest, RotateMemoryAndPhysical) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(physical(0), physical(1));
  expander.AddTask(physical(1), stack_slot(2));
  expander.AddTask(stack_slot(2), physical(0));
  expander.AddScratch(physical(2));
  EXPECT_EQ(
      "mov R2 = sp[2]\n"
      "mov sp[2] = R0\n"
      "mov R0 = R1\n"
      "mov R1 = R2\n",
      Expand(&expander));
}

TEST_F(LirParallelCopyExpanderTest, Swap) {
  ParallelCopyExpander expander(factory(), int32_type());
  expander.AddTask(physical(0), physical(1));
  expander.AddTask(physical(1), physical(0));
  EXPECT_EQ("pcopy R1, R0 = R0, R1\n", Expand(&expander));
}

}  // namespace
}  // namespace lir
}  // namespace elang
