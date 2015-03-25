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

TEST_F(EditorTest, SetInput) {
  auto const function = NewSampleFunction(bool_type(), int32_type());
  Editor editor(factory(), function);
  auto const ret_node = function->exit_node()->input(0);
  editor.SetInput(ret_node, 1, false_value());

  EXPECT_EQ(
      "function1 bool(int32)\n"
      "0000: (control, effect, int32) %t1 = Entry()\n"
      "0001: control %c2 = Ret(%t1, Bool(0))\n"
      "0002: void %r3 = Exit(%c2, %t1)\n",
      ToString(function));
}

}  // namespace optimizer
}  // namespace elang
