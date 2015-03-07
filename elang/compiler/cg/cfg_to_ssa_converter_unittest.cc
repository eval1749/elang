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
TEST_F(CfgToSsaTest, If) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() {\n"
      "     var x = 1;\n"
      "     if (Cond()) x = Bar(x); else x = Baz(x);\n"
      "     x = Baz(x);\n"
      "     return x;\n"
      "  }\n"
      "  static int Bar(int x) { return x; }\n"
      "  static int Baz(int x) { return x; }\n"
      "  static bool Cond() { return true; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block5\n"
      "  entry\n"
      "  int32* %p4 = alloca 1\n"
      "  store %p4, %p4, 1\n"
      "  bool %b6 = call `Sample.Cond`, void\n"
      "  br %b6, block4, block5\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r7 = load %p4, %p4\n"
      "  int32 %r8 = call `Sample.Bar`, %r7\n"
      "  store %p4, %p4, %r8\n"
      "  br block3\n"
      "block5:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r11 = load %p4, %p4\n"
      "  int32 %r12 = call `Sample.Baz`, %r11\n"
      "  store %p4, %p4, %r12\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block5\n"
      "  // Out: block2\n"
      "  int32 %r16 = load %p4, %p4\n"
      "  int32 %r17 = call `Sample.Baz`, %r16\n"
      "  store %p4, %p4, %r17\n"
      "  int32 %r19 = load %p4, %p4\n"
      "  ret %r19, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block5\n"
      "  entry\n"
      "  bool %b6 = call `Sample.Cond`, void\n"
      "  br %b6, block4, block5\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r8 = call `Sample.Bar`, 1\n"
      "  br block3\n"
      "block5:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r12 = call `Sample.Baz`, 1\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block5\n"
      "  // Out: block2\n"
      "  int32 %r29 = phi block5 %r12, block4 %r8\n"
      "  int32 %r17 = call `Sample.Baz`, %r29\n"
      "  ret %r17, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      ConvertToSsa("Sample.Foo"));
}

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
      "  int32* %p4 = alloca 1\n"
      "  store %p4, %p4, 1\n"
      "  int32 %r6 = load %p4, %p4\n"
      "  int32 %r7 = call `Sample.Bar`, %r6\n"
      "  store %p4, %p4, %r7\n"
      "  int32 %r9 = load %p4, %p4\n"
      "  int32 %r10 = call `Sample.Baz`, %r9\n"
      "  store %p4, %p4, %r10\n"
      "  int32 %r12 = load %p4, %p4\n"
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

TEST_F(CfgToSsaTest, Loop) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() {\n"
      "     for (var x = 1; Cond(); x = Bar(x)) {\n"
      "       x = Baz(x);\n"
      "     }\n"
      "     return 42;\n"
      "  }\n"
      "  static int Bar(int x) { return x; }\n"
      "  static int Baz(int x) { return x; }\n"
      "  static bool Cond() { return true; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4\n"
      "  entry\n"
      "  int32* %p5 = alloca 1\n"
      "  store %p5, %p5, 1\n"
      "  br block4\n"
      "block5:\n"
      "  // In: block4\n"
      "  // Out: block6\n"
      "  int32 %r8 = load %p5, %p5\n"
      "  int32 %r9 = call `Sample.Baz`, %r8\n"
      "  store %p5, %p5, %r9\n"
      "  br block6\n"
      "block6:\n"
      "  // In: block5\n"
      "  // Out: block4\n"
      "  int32 %r12 = load %p5, %p5\n"
      "  int32 %r13 = call `Sample.Bar`, %r12\n"
      "  store %p5, %p5, %r13\n"
      "  br block4\n"
      "block4:\n"
      "  // In: block1 block6\n"
      "  // Out: block5 block3\n"
      "  bool %b15 = call `Sample.Cond`, void\n"
      "  br %b15, block5, block3\n"
      "block3:\n"
      "  // In: block4\n"
      "  // Out: block2\n"
      "  ret 42, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4\n"
      "  entry\n"
      "  br block4\n"
      "block5:\n"
      "  // In: block4\n"
      "  // Out: block6\n"
      "  int32 %r9 = call `Sample.Baz`, %r26\n"
      "  br block6\n"
      "block6:\n"
      "  // In: block5\n"
      "  // Out: block4\n"
      "  int32 %r13 = call `Sample.Bar`, %r9\n"
      "  br block4\n"
      "block4:\n"
      "  // In: block1 block6\n"
      "  // Out: block5 block3\n"
      "  int32 %r26 = phi block1 1, block6 %r13\n"
      "  bool %b15 = call `Sample.Cond`, void\n"
      "  br %b15, block5, block3\n"
      "block3:\n"
      "  // In: block4\n"
      "  // Out: block2\n"
      "  ret 42, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      ConvertToSsa("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
