// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "elang/lir/testing/lir_test.h"

#include "base/logging.h"
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
  typedef std::pair<Value, Value> Task;

  LirParallelCopyExpanderTest() = default;
  ~LirParallelCopyExpanderTest() = default;

  static Value physical(int data) {
    return Value(Value::Type::Integer, ValueSize::Size32,
                 Value::Kind::PhysicalRegister, data);
  }

  static Value stack_slot(int data) {
    return Value(Value::Type::Integer, ValueSize::Size32,
                 Value::Kind::StackSlot, data);
  }

  void Expand(const std::vector<Task>& tasks, base::StringPiece expected);
  void ExpandWithScratch(const std::vector<Task>& tasks,
                         Value scratch,
                         base::StringPiece expected);
  void ExpandWithScratch2(const std::vector<Task>& tasks,
                          Value scratch1,
                          Value scratch2,
                          base::StringPiece expected);

 private:
  DISALLOW_COPY_AND_ASSIGN(LirParallelCopyExpanderTest);
};

void LirParallelCopyExpanderTest::Expand(const std::vector<Task>& tasks,
                                         base::StringPiece expected) {
  ExpandWithScratch2(tasks, Value::Void(), Value::Void(), expected);
}

void LirParallelCopyExpanderTest::ExpandWithScratch(
    const std::vector<Task>& tasks,
    Value scratch1,
    base::StringPiece expected) {
  ExpandWithScratch2(tasks, scratch1, Value::Void(), expected);
}

void LirParallelCopyExpanderTest::ExpandWithScratch2(
    const std::vector<Task>& original_tasks,
    Value scratch1,
    Value scratch2,
    base::StringPiece expected) {
  DCHECK(!original_tasks.empty());
  std::vector<int> indexes(original_tasks.size());
  for (auto index = 0u; index < indexes.size(); ++index)
    indexes[index] = index;
  do {
    ParallelCopyExpander expander(factory(), Value::Int32Type());
    std::vector<Task> tasks(indexes.size());
    tasks.resize(0);
    for (auto const index : indexes)
      tasks.push_back(original_tasks[index]);
    for (auto task : tasks)
      expander.AddTask(task.first, task.second);
    if (scratch1.is_physical())
      expander.AddScratch(scratch1);
    if (scratch2.is_physical())
      expander.AddScratch(scratch2);

    std::ostringstream ostream;
    for (auto const instr : expander.Expand())
      ostream << PrintAsGeneric(instr) << std::endl;
    EXPECT_EQ(expected, ostream.str());

    std::rotate(tasks.begin(), tasks.begin() + 1, tasks.end());
  } while (std::next_permutation(indexes.begin(), indexes.end()));
}

// Test cases...

// M0 <- M2, M1 <- r0, r1 <- I0
TEST_F(LirParallelCopyExpanderTest, AutoScratchByImmediate) {
  Expand(
      {
       std::make_pair(stack_slot(0), stack_slot(2)),
       std::make_pair(stack_slot(1), physical(0)),
       std::make_pair(physical(1), Value::SmallInt32(42)),
      },
      "mov r1 = sp[2]\n"
      "mov sp[0] = r1\n"
      "mov sp[1] = r0\n"
      "lit r1 = #42\n");
}

// M0, r1 <- M2, M1 <- M0
TEST_F(LirParallelCopyExpanderTest, AutoScratchByMemory) {
  Expand(
      {
       std::make_pair(stack_slot(0), stack_slot(2)),
       std::make_pair(stack_slot(1), physical(0)),
       std::make_pair(physical(1), stack_slot(2)),
      },
      "mov r1 = sp[2]\n"
      "mov sp[0] = r1\n"
      "mov sp[1] = r0\n");
}

// M0 <- M2, M1 <- r0, r1 <- M3
TEST_F(LirParallelCopyExpanderTest, AutoScratchByMemory2) {
  Expand(
      {
       std::make_pair(stack_slot(0), stack_slot(2)),
       std::make_pair(stack_slot(1), physical(0)),
       std::make_pair(physical(1), stack_slot(3)),
      },
      "mov r1 = sp[2]\n"
      "mov sp[0] = r1\n"
      "mov sp[1] = r0\n"
      "mov r1 = sp[3]\n");
}

// r0 <- M1 <- r0, M2 <- r3; we can use M2 as spill location for r0.
TEST_F(LirParallelCopyExpanderTest, AutoScratchFromStore) {
  Expand(
      {
       std::make_pair(physical(0), stack_slot(1)),
       std::make_pair(stack_slot(1), physical(0)),
       std::make_pair(stack_slot(2), physical(3)),
      },
      "mov sp[2] = r3\n"  // store r3
      "mov r3 = sp[1]\n"
      "mov sp[1] = r0\n"
      "mov r0 = r3\n"
      "mov r3 = sp[2]\n");  // reload r3
}

// r0 <- r1 <- r0, M2 <- r1
TEST_F(LirParallelCopyExpanderTest, AutoScratchFromSwap) {
  Expand(
      {
       std::make_pair(physical(0), physical(1)),
       std::make_pair(physical(1), physical(0)),
       std::make_pair(stack_slot(2), physical(1)),
      },
      "mov sp[2] = r1\n"
      "pcopy r0, r1 = r1, r0\n");
}

// r0, r2 <- r1, r4 <- r3
TEST_F(LirParallelCopyExpanderTest, Basic) {
  Expand(
      {
       std::make_pair(physical(0), physical(1)),
       std::make_pair(physical(2), physical(1)),
       std::make_pair(physical(4), physical(3)),
      },
      "mov r0 = r1\n"
      "mov r2 = r1\n"
      "mov r4 = r3\n");
}

// M0 <- M1 <- M0
TEST_F(LirParallelCopyExpanderTest, MemorySwap) {
  ExpandWithScratch2(
      {
       std::make_pair(stack_slot(0), stack_slot(1)),
       std::make_pair(stack_slot(1), stack_slot(0)),
      },
      physical(2), physical(3),
      "mov r3 = sp[1]\n"
      "mov r2 = sp[0]\n"
      "mov sp[0] = r3\n"
      "mov sp[1] = r2\n");
}

// memory swap requires 2 scratch register
TEST_F(LirParallelCopyExpanderTest, MemorySwapNoScratch) {
  Expand(
      {
       std::make_pair(stack_slot(0), stack_slot(1)),
       std::make_pair(stack_slot(1), stack_slot(0)),
      },
      "");
}

// memory swap requires 2 scratch register
TEST_F(LirParallelCopyExpanderTest, MemorySwapOneScratch) {
  Expand(
      {
       std::make_pair(stack_slot(0), stack_slot(1)),
       std::make_pair(stack_slot(1), stack_slot(0)),
      },
      "");
}

// M0 <- r0, M1 <- r1
TEST_F(LirParallelCopyExpanderTest, PhysicalToMemory) {
  Expand(
      {
       std::make_pair(stack_slot(0), physical(0)),
       std::make_pair(stack_slot(1), physical(1)),
      },
      "mov sp[0] = r0\n"
      "mov sp[1] = r1\n");
}

// r0 <- r1 <- r2 <- r0
TEST_F(LirParallelCopyExpanderTest, Rotate) {
  Expand(
      {
       std::make_pair(physical(0), physical(1)),
       std::make_pair(physical(1), physical(2)),
       std::make_pair(physical(2), physical(0)),
      },
      "pcopy r0, r1 = r1, r0\n"
      "pcopy r1, r2 = r2, r1\n");
}

// M0 <- M1 <- M2 <- M0
TEST_F(LirParallelCopyExpanderTest, RotateMemory) {
  ExpandWithScratch2(
      {
       std::make_pair(stack_slot(0), stack_slot(1)),
       std::make_pair(stack_slot(1), stack_slot(2)),
       std::make_pair(stack_slot(2), stack_slot(0)),
      },
      physical(4), physical(5),
      "mov r5 = sp[1]\n"
      "mov r4 = sp[0]\n"
      "mov sp[0] = r5\n"
      "mov r5 = sp[2]\n"
      "mov sp[1] = r5\n"
      "mov sp[2] = r4\n");
}

// r0 <- r1 <- M2 <- r0
TEST_F(LirParallelCopyExpanderTest, RotateMemoryAndPhysical) {
  ExpandWithScratch(
      {
       std::make_pair(physical(0), physical(1)),
       std::make_pair(physical(1), stack_slot(2)),
       std::make_pair(stack_slot(2), physical(0)),
      },
      physical(3),
      "mov r3 = sp[2]\n"
      "mov sp[2] = r0\n"
      "mov r0 = r1\n"
      "mov r1 = r3\n");
}

// r0 <- M1 <- M2 <- r0
TEST_F(LirParallelCopyExpanderTest, RotateMemoryAndPhysical2) {
  ExpandWithScratch(
      {
       std::make_pair(physical(0), stack_slot(1)),
       std::make_pair(stack_slot(1), stack_slot(2)),
       std::make_pair(stack_slot(2), physical(0)),
      },
      physical(3),
      "mov r3 = sp[2]\n"
      "mov sp[2] = r0\n"
      "mov r0 = sp[1]\n"
      "mov sp[1] = r3\n");
}

// r0 <- r1 <- M2 <- M3 <- r0
TEST_F(LirParallelCopyExpanderTest, RotateMemoryAndPhysical3) {
  ExpandWithScratch(
      {
       std::make_pair(physical(0), physical(1)),
       std::make_pair(physical(1), stack_slot(2)),
       std::make_pair(stack_slot(2), stack_slot(3)),
       std::make_pair(stack_slot(3), physical(0)),
      },
      physical(4),
      "mov r4 = sp[3]\n"
      "mov sp[3] = r0\n"
      "mov r0 = r1\n"
      "mov r1 = sp[2]\n"
      "mov sp[2] = r4\n");
}

// r0 <- r1 <- r0
TEST_F(LirParallelCopyExpanderTest, Swap) {
  Expand(
      {
       std::make_pair(physical(0), physical(1)),
       std::make_pair(physical(1), physical(0)),
      },
      "pcopy r0, r1 = r1, r0\n");
}

// r0 <- r1 <- r0, M2 <- M3 <- M2
TEST_F(LirParallelCopyExpanderTest, TwoCycles) {
  ExpandWithScratch2(
      {
       std::make_pair(physical(0), physical(1)),
       std::make_pair(physical(1), physical(0)),
       std::make_pair(stack_slot(2), stack_slot(3)),
       std::make_pair(stack_slot(3), stack_slot(2)),
      },
      physical(4), physical(5),
      "pcopy r0, r1 = r1, r0\n"
      "mov r5 = sp[3]\n"
      "mov r4 = sp[2]\n"
      "mov sp[2] = r5\n"
      "mov sp[3] = r4\n");
}

// r0 <- M1 <- r0, r2 <- M3 <- r2
TEST_F(LirParallelCopyExpanderTest, TwoCycles2) {
  ExpandWithScratch(
      {
       std::make_pair(physical(0), stack_slot(1)),
       std::make_pair(stack_slot(1), physical(0)),
       std::make_pair(physical(2), stack_slot(3)),
       std::make_pair(stack_slot(3), physical(2)),
      },
      physical(4),
      "mov r4 = sp[1]\n"
      "mov sp[1] = r0\n"
      "mov r0 = r4\n"
      "mov r4 = sp[3]\n"
      "mov sp[3] = r2\n"
      "mov r2 = r4\n");
}

// r0 <- M1 <- M2 <- r0, r3 <- M4 <- r3
TEST_F(LirParallelCopyExpanderTest, TwoCycles3) {
  ExpandWithScratch(
      {
       std::make_pair(physical(0), stack_slot(1)),
       std::make_pair(stack_slot(1), physical(0)),
       std::make_pair(stack_slot(2), physical(0)),
       std::make_pair(physical(3), stack_slot(4)),
       std::make_pair(stack_slot(4), physical(3)),
      },
      physical(5),
      "mov sp[2] = r0\n"
      "mov r5 = sp[1]\n"
      "mov sp[1] = r0\n"
      "mov r0 = r5\n"
      "mov r5 = sp[4]\n"
      "mov sp[4] = r3\n"
      "mov r3 = r5\n");
}

// r0 <- M1 <- r0, M2 <- M3 <- M2
TEST_F(LirParallelCopyExpanderTest, TwoCycles4) {
  ExpandWithScratch2(
      {
       std::make_pair(physical(0), stack_slot(1)),
       std::make_pair(stack_slot(1), physical(0)),
       std::make_pair(stack_slot(2), stack_slot(3)),
       std::make_pair(stack_slot(3), stack_slot(2)),
      },
      physical(4), physical(5),
      "mov r5 = sp[1]\n"
      "mov sp[1] = r0\n"
      "mov r0 = r5\n"
      "mov r5 = sp[3]\n"
      "mov r4 = sp[2]\n"
      "mov sp[2] = r5\n"
      "mov sp[3] = r4\n");
}

}  // namespace
}  // namespace lir
}  // namespace elang
