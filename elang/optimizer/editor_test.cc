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

TEST_F(EditorTest, BuildIf) {
  auto const function = NewSampleFunction(int32_type(), bool_type());
  Editor editor(factory(), function);
  auto const param0 = editor.EmitParameter(0);
  editor.StartIf(param0);

  editor.StartThen();
  editor.EndWithRet(NewInt32(42));

  editor.StartElse();
  editor.EndWithRet(NewInt32(33));

  editor.EndIf();

  // TODO(eval1749) Make |ExitNode| to take two inputs, control and effect.
  // TODO(eval1749) Make |NewFunction| to populate |EntryNode| and |ExitNode|
  // only.
  EXPECT_EQ(
      "function1 int32(bool)\n"
      "0000: (control, effect, bool) %t1 = entry()\n"
      "0001: bool %r3 = param(%t1, 0)\n"
      "0002: control %c4 = if(%t1, %r3)\n"
      "0003: control %c5 = if_true(%c4)\n"
      "0004: control %c6 = ret(%c5, lit_i32(42))\n"
      "0005: control %c8 = if_false(%c4)\n"
      "0006: control %c9 = ret(%c8, lit_i32(33))\n"
      "0007: control %c7 = merge(%t1, %c6, %c9)\n"
      "0008: void %r2 = exit(%c7, %t1)\n",
      ToString(function));
}

TEST_F(EditorTest, EndWithRet) {
  auto const function = NewSampleFunction(int32_type(), int32_type());
  Editor editor(factory(), function);
  editor.EndWithRet(NewInt32(42));

  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: (control, effect, int32) %t1 = entry()\n"
      "0001: control %c3 = ret(%t1, lit_i32(42))\n"
      "0002: void %r2 = exit(%c3, %t1)\n",
      ToString(function));
}

TEST_F(EditorTest, SetInput) {
  auto const function = NewSampleFunction(int32_type(), int32_type());
  Editor editor(factory(), function);
  editor.EndWithRet(NewInt32(42));
  auto const ret_node = function->exit_node()->input(0);
  editor.SetInput(ret_node, 1, NewInt32(33));

  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: (control, effect, int32) %t1 = entry()\n"
      "0001: control %c3 = ret(%t1, lit_i32(33))\n"
      "0002: void %r2 = exit(%c3, %t1)\n",
      ToString(function));
}

}  // namespace optimizer
}  // namespace elang
