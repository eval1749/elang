// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// NodeTest
//
class NodeTest : public testing::OptimizerTest {
 protected:
  NodeTest() = default;
  ~NodeTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeTest);
};

TEST_F(NodeTest, Function) {
  auto const function = NewFunction(NewFunctionType(void_type(), void_type()));
  EXPECT_EQ("(control, effect, void) %t1 = Entry()",
            ToString(function->entry_node()));
  EXPECT_EQ("void %r3 = Exit(%c2, %t1)", ToString(function->exit_node()));
}

}  // namespace optimizer
}  // namespace elang
