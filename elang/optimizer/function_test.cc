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
  auto const function = NewSampleFunction(void_type(), void_type());
  // Note: There is no entry node in expected result, because
  // |Factory::NewFunction()| doesn't connect entry and exit nodes.
  EXPECT_EQ(
      "function1 void(void)\n"
      "0000: control %c2 = merge()\n"
      "0001: void %r3 = exit(%c2)\n",
      ToString(function));
}

}  // namespace optimizer
}  // namespace elang
