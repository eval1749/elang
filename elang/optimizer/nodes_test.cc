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
// NodeTest
//
class NodeTest : public testing::OptimizerTest {
 protected:
  NodeTest() = default;
  ~NodeTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeTest);
};

TEST_F(NodeTest, EntryNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = function->entry_node();
  EXPECT_EQ("(control, effect, void) %t1 = Entry()", ToString(node));
}

TEST_F(NodeTest, ExitNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = function->exit_node();
  EXPECT_EQ("void %r3 = Exit(%c2, %t1)", ToString(node));
}

TEST_F(NodeTest, RetNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = function->exit_node()->input(0);
  EXPECT_EQ("control %c2 = Ret(%t1, void)", ToString(node));
}

}  // namespace optimizer
}  // namespace elang
