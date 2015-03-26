// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"

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
  EXPECT_EQ("(control, effect, void) %t1 = entry()", ToString(node));
}

TEST_F(NodeTest, ExitNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = function->exit_node();
  EXPECT_EQ("void %r2 = exit(%t1, %t1)", ToString(node));
}

TEST_F(NodeTest, GetNode) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const node = NewGet(entry_node, 2);
  EXPECT_EQ("(int32, int64) %t3 = get(%t1, 2)", ToString(node));
}

TEST_F(NodeTest, RetNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const entry_node = function->entry_node();
  auto const node = NewRet(entry_node, void_value());
  EXPECT_EQ("control %c3 = ret(%t1, void)", ToString(node));
}

TEST_F(NodeTest, ParameterNode) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const entry_node = function->entry_node();
  auto const node = NewParameter(entry_node, 0);
  EXPECT_EQ("int32 %r3 = param(%t1, 0)", ToString(node));
}

TEST_F(NodeTest, ParameterNode2) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const node = NewParameter(entry_node, 1);
  EXPECT_EQ("int64 %r3 = param(%t1, 1)", ToString(node));
}

}  // namespace optimizer
}  // namespace elang
