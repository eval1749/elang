// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/editor.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// EditorTest
//
class EditorTest : public testing::OptimizerTest {
 protected:
  EditorTest() = default;
  ~EditorTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(EditorTest);
};

TEST_F(EditorTest, ChangeInput) {
  auto const function = NewSampleFunction(int32_type(), int32_type());
  Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewEffectGet(entry_node, 1);

  editor.Edit(NewControlGet(entry_node, 0));
  editor.SetRet(effect, NewInt32(42));

  auto const ret_node = function->exit_node()->input(0)->input(0);
  editor.ChangeInput(ret_node, 2, NewInt32(33));

  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: (control, effect, int32) %t1 = entry()\n"
      "0001: control %c5 = control_get(%t1, 0)\n"
      "0002: effect %e4 = effect_get(%t1, 1)\n"
      "0003: control %c6 = ret(%c5, %e4, 33)\n"
      "0004: control %c2 = merge(%c6)\n"
      "0005: void %r3 = exit(%c2)\n",
      ToString(function));
}

TEST_F(EditorTest, SetBranch) {
  auto const function = NewSampleFunction(int32_type(), bool_type());
  Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewEffectGet(entry_node, 1);

  editor.Edit(NewControlGet(entry_node, 0));
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
      "0000: (control, effect, bool) %t1 = entry()\n"
      "0001: control %c5 = control_get(%t1, 0)\n"
      "0002: bool %r6 = param(%t1, 0)\n"
      "0003: control %c7 = if(%c5, %r6)\n"
      "0004: control %c8 = if_true(%c7)\n"
      "0005: effect %e4 = effect_get(%t1, 1)\n"
      "0006: control %c10 = ret(%c8, %e4, 42)\n"
      "0007: control %c9 = if_false(%c7)\n"
      "0008: control %c11 = ret(%c9, %e4, 33)\n"
      "0009: control %c2 = merge(%c10, %c11)\n"
      "0010: void %r3 = exit(%c2)\n",
      ToString(function));
}

TEST_F(EditorTest, SetBranchPhi) {
  auto const function = NewSampleFunction(
      int32_type(), NewTupleType({bool_type(), int32_type(), int32_type()}));
  Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewEffectGet(entry_node, 1);

  editor.Edit(NewControlGet(entry_node, 0));
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
      "0000: (control, effect, (bool, int32, int32)) %t1 = entry()\n"
      "0001: control %c5 = control_get(%t1, 0)\n"
      "0002: bool %r6 = param(%t1, 0)\n"
      "0003: control %c7 = if(%c5, %r6)\n"
      "0004: control %c8 = if_true(%c7)\n"
      "0005: control %c11 = br(%c8)\n"
      "0006: control %c9 = if_false(%c7)\n"
      "0007: control %c12 = br(%c9)\n"
      "0008: control %c10 = merge(%c11, %c12)\n"
      "0009: effect %e4 = effect_get(%t1, 1)\n"
      "0010: int32 %r14 = param(%t1, 1)\n"
      "0011: int32 %r15 = param(%t1, 2)\n"
      "0012: int32 %r13 = phi(%c11: %r14, %c12: %r15)\n"
      "0013: control %c16 = ret(%c10, %e4, %r13)\n"
      "0014: control %c2 = merge(%c16)\n"
      "0015: void %r3 = exit(%c2)\n",
      ToString(function));
}

TEST_F(EditorTest, SetRet) {
  auto const function = NewSampleFunction(int32_type(), int32_type());
  Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewEffectGet(entry_node, 1);

  editor.Edit(NewControlGet(entry_node, 0));
  editor.SetRet(effect, NewInt32(42));
  editor.Commit();

  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: (control, effect, int32) %t1 = entry()\n"
      "0001: control %c5 = control_get(%t1, 0)\n"
      "0002: effect %e4 = effect_get(%t1, 1)\n"
      "0003: control %c6 = ret(%c5, %e4, 42)\n"
      "0004: control %c2 = merge(%c6)\n"
      "0005: void %r3 = exit(%c2)\n",
      ToString(function));
}

}  // namespace optimizer
}  // namespace elang
