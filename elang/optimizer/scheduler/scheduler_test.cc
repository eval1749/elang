// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/editor.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/optimizer/scheduler/scheduler.h"
#include "elang/optimizer/scheduler/schedule_formatter.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// SchedulerTest
//
class SchedulerTest : public testing::OptimizerTest {
 protected:
  SchedulerTest() = default;
  ~SchedulerTest() override = default;

  std::string ScheduleOf(Function* function);

 private:
  DISALLOW_COPY_AND_ASSIGN(SchedulerTest);
};

std::string SchedulerTest::ScheduleOf(Function* function) {
  Schedule schedule(function);
  Scheduler scheduler(&schedule);
  scheduler.Run();
  std::stringstream ostream;
  ostream << AsFormatted(schedule);
  return ostream.str();
}

TEST_F(SchedulerTest, SetBranch) {
  auto const function = NewSampleFunction(int32_type(), bool_type());
  Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const param0 = editor.ParameterAt(0);
  auto const if_node = editor.SetBranch(param0);
  auto const if_true = NewIfTrue(if_node);
  auto const if_false = NewIfFalse(if_node);
  editor.Commit();

  editor.Edit(if_true);
  editor.SetRet(effect, NewInt32(42));
  editor.Commit();

  editor.Edit(if_false);
  editor.SetRet(effect, NewInt32(33));
  editor.Commit();

  EXPECT_EQ(
      "function1 int32(bool)\n"
      "block1:\n"
      "  in: {}\n"
      "  out: {block7, block8}\n"
      "0000: control(bool) %c1 = entry()\n"
      "0001: bool %r5 = param(%c1, 0)\n"
      "0002: effect %e4 = get_effect(%c1)\n"
      "0003: control %c6 = if(%c1, %r5)\n"
      "block8:\n"
      "  in: {block1}\n"
      "  out: {block2}\n"
      "0004: control %c8 = if_false(%c6)\n"
      "0005: control %c10 = ret(%c8, %e4, 33)\n"
      "block7:\n"
      "  in: {block1}\n"
      "  out: {block2}\n"
      "0006: control %c7 = if_true(%c6)\n"
      "0007: control %c9 = ret(%c7, %e4, 42)\n"
      "block2:\n"
      "  in: {block7, block8}\n"
      "  out: {}\n"
      "0008: control %c2 = merge(%c9, %c10)\n"
      "0009: exit(%c2)\n",
      ScheduleOf(function));
}

TEST_F(SchedulerTest, SetBranchPhi) {
  auto const function = NewSampleFunction(
      int32_type(), NewTupleType({bool_type(), int32_type(), int32_type()}));
  Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const if_node = editor.SetBranch(NewParameter(entry_node, 0));
  auto const if_true = NewIfTrue(if_node);
  auto const if_false = NewIfFalse(if_node);
  editor.Commit();

  auto const ret_control = NewMerge({});

  editor.Edit(if_true);
  editor.SetJump(ret_control);
  editor.Commit();

  editor.Edit(if_false);
  editor.SetJump(ret_control);
  editor.Commit();

  editor.Edit(ret_control);
  auto const phi = NewPhi(int32_type(), ret_control);
  editor.SetPhiInput(phi, ret_control->control(0), NewParameter(entry_node, 1));
  editor.SetPhiInput(phi, ret_control->control(1), NewParameter(entry_node, 2));
  editor.SetRet(effect, phi);
  editor.Commit();

  EXPECT_EQ(
      "function1 int32(bool, int32, int32)\n"
      "block1:\n"
      "  in: {}\n"
      "  out: {block7, block8}\n"
      "0000: control((bool, int32, int32)) %c1 = entry()\n"
      "0001: bool %r5 = param(%c1, 0)\n"
      "0002: effect %e4 = get_effect(%c1)\n"
      "0003: int32 %r13 = param(%c1, 1)\n"
      "0004: int32 %r14 = param(%c1, 2)\n"
      "0005: control %c6 = if(%c1, %r5)\n"
      "block8:\n"
      "  in: {block1}\n"
      "  out: {block9}\n"
      "0006: control %c8 = if_false(%c6)\n"
      "0007: control %c11 = br(%c8)\n"
      "block7:\n"
      "  in: {block1}\n"
      "  out: {block9}\n"
      "0008: control %c7 = if_true(%c6)\n"
      "0009: control %c10 = br(%c7)\n"
      "block9:\n"
      "  in: {block7, block8}\n"
      "  out: {block2}\n"
      "0010: control %c9 = merge(%c10, %c11)\n"
      "0011: int32 %r12 = phi(%c10: %r13, %c11: %r14)\n"
      "0012: control %c15 = ret(%c9, %e4, %r12)\n"
      "block2:\n"
      "  in: {block9}\n"
      "  out: {}\n"
      "0013: control %c2 = merge(%c15)\n"
      "0014: exit(%c2)\n",
      ScheduleOf(function));
}

TEST_F(SchedulerTest, SetRet) {
  auto const function = NewSampleFunction(int32_type(), int32_type());
  Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect, NewInt32(42));
  editor.Commit();

  EXPECT_EQ(
      "function1 int32(int32)\n"
      "block1:\n"
      "  in: {}\n"
      "  out: {block2}\n"
      "0000: control(int32) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: control %c5 = ret(%c1, %e4, 42)\n"
      "block2:\n"
      "  in: {block1}\n"
      "  out: {}\n"
      "0003: control %c2 = merge(%c5)\n"
      "0004: exit(%c2)\n",
      ScheduleOf(function));
}

}  // namespace optimizer
}  // namespace elang
