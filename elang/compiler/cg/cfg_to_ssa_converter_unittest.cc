// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/cg_test.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaTest
//
class CfgToSsaTest : public testing::CgTest {
 protected:
  CfgToSsaTest() = default;
  ~CfgToSsaTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(CfgToSsaTest);
};

//////////////////////////////////////////////////////////////////////
//
// Tests...
//
TEST_F(CfgToSsaTest, Local) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() {\n"
      "     var x = 1;\n"
      "     x = Bar(x);\n"
      "     x = Baz(x);\n"
      "     return x;\n"
      "  }\n"
      "  static int Bar(int x) { return x; }\n"
      "  static int Baz(int x) { return x; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  int32* %r4 = call `StackAlloc`, void\n"
      "  store %r4, 1\n"
      "  int32 %r6 = load %r4\n"
      "  int32 %r7 = call `Sample.Bar`, %r6\n"
      "  store %r4, %r7\n"
      "  int32 %r9 = load %r4\n"
      "  int32 %r10 = call `Sample.Baz`, %r9\n"
      "  store %r4, %r10\n"
      "  int32 %r12 = load %r4\n"
      "  ret %r12, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  int32 %r7 = call `Sample.Bar`, 1\n"
      "  int32 %r10 = call `Sample.Baz`, %r7\n"
      "  ret %r10, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      ConvertToSsa("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
