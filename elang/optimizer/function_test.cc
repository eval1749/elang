// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// FunctionTest
//
class FunctionTest : public testing::OptimizerTest {
 protected:
  FunctionTest() = default;
  ~FunctionTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(FunctionTest);
};

TEST_F(FunctionTest, Function) {
  auto const function = NewFunction(NewFunctionType(void_type(), void_type()));
  EXPECT_EQ("(control, effect, void) %t1 = Entry()",
            ToString(function->entry_node()));
  EXPECT_EQ("void %r3 = Exit(%c2, %t1)", ToString(function->exit_node()));

  EXPECT_EQ(
      "function1 void(void)\n"
      "0000: (control, effect, void) %t1 = Entry()\n"
      "0001: control %c2 = Ret(%t1, void)\n"
      "0002: void %r3 = Exit(%c2, %t1)\n",
      ToString(function));
}

}  // namespace optimizer
}  // namespace elang
