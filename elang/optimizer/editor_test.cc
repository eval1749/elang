// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/editor.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"

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

  editor.Edit(function->entry_node());
  editor.SetRet(NewInt32(42));

  auto const ret_node = function->exit_node()->input(0)->input(0);
  editor.ChangeInput(ret_node, 2, NewInt32(33));

  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: (control, effect, int32) %t1 = entry()\n"
      "0001: control %c2 = get(%t1, 0)\n"
      "0002: effect %e3 = get(%t1, 1)\n"
      "0003: control %c6 = ret(%c2, %e3, 33)\n"
      "0004: control %c4 = merge(%c6)\n"
      "0005: void %r5 = exit(%c4)\n",
      ToString(function));
}

TEST_F(EditorTest, SetIf) {
  auto const function = NewSampleFunction(int32_type(), bool_type());
  Editor editor(factory(), function);

  editor.Edit(function->entry_node());
  auto const param0 = editor.EmitParameter(0);
  auto const merge_control = editor.SetBranch(param0);
  editor.Commit();

  editor.Edit(merge_control->input(0));
  editor.SetRet(NewInt32(42));
  editor.Commit();

  editor.Edit(merge_control->input(1));
  editor.SetRet(NewInt32(33));
  editor.Commit();

  EXPECT_EQ(
      "function1 int32(bool)\n"
      "0000: (control, effect, bool) %t1 = entry()\n"
      "0001: control %c2 = get(%t1, 0)\n"
      "0002: bool %r6 = param(%t1, 0)\n"
      "0003: control %c7 = if(%c2, %r6)\n"
      "0004: control %c8 = if_true(%c7)\n"
      "0005: effect %e3 = get(%t1, 1)\n"
      "0006: control %c11 = ret(%c8, %e3, 42)\n"
      "0007: control %c9 = if_false(%c7)\n"
      "0008: control %c12 = ret(%c9, %e3, 33)\n"
      "0009: control %c4 = merge(%c11, %c12)\n"
      "0010: void %r5 = exit(%c4)\n",
      ToString(function));
}

TEST_F(EditorTest, SetRet) {
  auto const function = NewSampleFunction(int32_type(), int32_type());
  Editor editor(factory(), function);

  editor.Edit(function->entry_node());
  editor.SetRet(NewInt32(42));
  editor.Commit();

  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: (control, effect, int32) %t1 = entry()\n"
      "0001: control %c2 = get(%t1, 0)\n"
      "0002: effect %e3 = get(%t1, 1)\n"
      "0003: control %c6 = ret(%c2, %e3, 42)\n"
      "0004: control %c4 = merge(%c6)\n"
      "0005: void %r5 = exit(%c4)\n",
      ToString(function));
}

}  // namespace optimizer
}  // namespace elang
