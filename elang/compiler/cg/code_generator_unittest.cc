// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/cg_test.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// CodeGeneratorTest
//
class CodeGeneratorTest : public testing::CgTest {
 protected:
  CodeGeneratorTest() = default;
  ~CodeGeneratorTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(CodeGeneratorTest);
};

//////////////////////////////////////////////////////////////////////
//
// Tests...
//
TEST_F(CodeGeneratorTest, Assignment) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(int x) { x = Bar(); return x; }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  int32 %r2 = entry\n"
      "  int32* %r4 = call `StackAlloc`, void\n"
      "  store %r4, %r2\n"
      "  int32 %r6 = call `Sample.Bar`, void\n"
      "  store %r4, %r6\n"
      "  int32 %r8 = load %r4\n"
      "  ret %r8, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

// Binary Operations
TEST_F(CodeGeneratorTest, BinaryOperation) {
  Prepare(
      "class Sample {"
      "  static int Foo(int x) {"
      "    var w = x + 12;"
      "    return w * 34;"
      "  }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  int32 %r2 = entry\n"
      "  int32 %r4 = add %r2, 12\n"
      "  int32 %r5 = mul %r4, 34\n"
      "  ret %r5, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, BinaryOperationConditional) {
  Prepare(
      "class Sample {"
      "  static int Foo(bool x) {"
      "    return x && True() || False() ? 12 : 34;"
      "  }"
      "  static bool True() { return true; }"
      "  static bool False() { return false; }"
      "}");
  EXPECT_EQ(
      "function1 int32(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block3\n"
      "  bool %b2 = entry\n"
      "  br %b2, block4, block3\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  bool %b4 = call `Sample.True`, void\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block1\n"
      "  // Out: block5 block6\n"
      "  bool %b7 = phi block1 %b2, block4 %b4\n"
      "  br %b7, block5, block6\n"
      "block6:\n"
      "  // In: block3\n"
      "  // Out: block5\n"
      "  bool %b8 = call `Sample.False`, void\n"
      "  br block5\n"
      "block5:\n"
      "  // In: block6 block3\n"
      "  // Out: block8 block9\n"
      "  bool %b11 = phi block3 %b7, block6 %b8\n"
      "  br %b11, block8, block9\n"
      "block8:\n"
      "  // In: block5\n"
      "  // Out: block7\n"
      "  br block7\n"
      "block9:\n"
      "  // In: block5\n"
      "  // Out: block7\n"
      "  br block7\n"
      "block7:\n"
      "  // In: block8 block9\n"
      "  // Out: block2\n"
      "  int32 %r15 = phi block8 12, block9 34\n"
      "  ret %r15, block2\n"
      "block2:\n"
      "  // In: block7\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Call) {
  Prepare(
      "using System;\n"
      "class Sample {\n"
      "  static void Foo() { Bar(123); }\n"
      "  static void Bar(int x) {}\n"
      "}\n");
  EXPECT_EQ(
      "function1 void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  call `Sample.Bar`, 123\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Conditional) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(bool x) { var y = x ? Bar() : 38; return y; }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block5\n"
      "  bool %b2 = entry\n"
      "  br %b2, block4, block5\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r4 = call `Sample.Bar`, void\n"
      "  br block3\n"
      "block5:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block5\n"
      "  // Out: block2\n"
      "  int32 %r8 = phi block4 %r4, block5 38\n"
      "  ret %r8, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Do) {
  Prepare(
      "class Sample {\n"
      "  static void Foo(bool x) {\n"
      "    do {\n"
      "      if (x) break;\n"
      "      if (Bar()) continue;\n"
      "    } while (Baz());\n"
      "  }\n"
      "  static bool Bar() { return true; }\n"
      "  static bool Baz() { return true; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 void(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block5\n"
      "  bool %b2 = entry\n"
      "  br block5\n"
      "block5:\n"
      "  // In: block1 block9 block4\n"
      "  // Out: block7 block6\n"
      "  br %b2, block7, block6\n"
      "block7:\n"
      "  // In: block5\n"
      "  // Out: block3\n"
      "  br block3\n"
      "block6:\n"
      "  // In: block5\n"
      "  // Out: block9 block8\n"
      "  bool %b8 = call `Sample.Bar`, void\n"
      "  br %b8, block9, block8\n"
      "block9:\n"
      "  // In: block6\n"
      "  // Out: block5\n"
      "  br block5\n"
      "block8:\n"
      "  // In: block6\n"
      "  // Out: block4\n"
      "  br block4\n"
      "block4:\n"
      "  // In: block8\n"
      "  // Out: block5 block3\n"
      "  bool %b11 = call `Sample.Baz`, void\n"
      "  br %b11, block5, block3\n"
      "block3:\n"
      "  // In: block7 block4\n"
      "  // Out: block2\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Empty) {
  Prepare(
      "class Sample {\n"
      "  static void Foo() {}\n"
      "}\n");
  EXPECT_EQ(
      "function1 void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, EmptyError) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() {}\n"
      "}\n");
  EXPECT_EQ("CodeGenerator.Return.None(28) Foo\n", Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, For) {
  Prepare(
      "class Sample {\n"
      "  static void Foo(bool x) {\n"
      "    for (Init(); Baz(); Step()) {\n"
      "      if (x) break;\n"
      "      if (Bar()) continue;\n"
      "    }\n"
      "  }\n"
      "  static bool Bar() { return true; }\n"
      "  static bool Baz() { return true; }\n"
      "  static void Init() {}\n"
      "  static void Step() {}\n"
      "}\n");
  EXPECT_EQ(
      "function1 void(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4\n"
      "  bool %b2 = entry\n"
      "  call `Sample.Init`, void\n"
      "  br block4\n"
      "block5:\n"
      "  // In: block4\n"
      "  // Out: block8 block7\n"
      "  br %b2, block8, block7\n"
      "block8:\n"
      "  // In: block5\n"
      "  // Out: block3\n"
      "  br block3\n"
      "block7:\n"
      "  // In: block5\n"
      "  // Out: block10 block9\n"
      "  bool %b9 = call `Sample.Bar`, void\n"
      "  br %b9, block10, block9\n"
      "block10:\n"
      "  // In: block7\n"
      "  // Out: block6\n"
      "  br block6\n"
      "block9:\n"
      "  // In: block7\n"
      "  // Out: block6\n"
      "  br block6\n"
      "block6:\n"
      "  // In: block9 block10\n"
      "  // Out: block4\n"
      "  call `Sample.Step`, void\n"
      "  br block4\n"
      "block4:\n"
      "  // In: block1 block6\n"
      "  // Out: block5 block3\n"
      "  bool %b14 = call `Sample.Baz`, void\n"
      "  br %b14, block5, block3\n"
      "block3:\n"
      "  // In: block8 block4\n"
      "  // Out: block2\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, If) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(bool x) {\n"
      "    if (x) Bar();\n"
      "    if (x)\n"
      "      return 3;\n"
      "    else\n"
      "      Bar();\n"
      "    return 56;\n"
      "  }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block3\n"
      "  bool %b2 = entry\n"
      "  br %b2, block4, block3\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r4 = call `Sample.Bar`, void\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block1\n"
      "  // Out: block6 block7\n"
      "  br %b2, block6, block7\n"
      "block6:\n"
      "  // In: block3\n"
      "  // Out: block2\n"
      "  ret 3, block2\n"
      "block7:\n"
      "  // In: block3\n"
      "  // Out: block5\n"
      "  int32 %r8 = call `Sample.Bar`, void\n"
      "  br block5\n"
      "block5:\n"
      "  // In: block7\n"
      "  // Out: block2\n"
      "  ret 56, block2\n"
      "block2:\n"
      "  // In: block5 block6\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Parameter) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(int x) { return x; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  int32 %r2 = entry\n"
      "  ret %r2, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

// 'return' statement
TEST_F(CodeGeneratorTest, Return) {
  Prepare(
      "class Sample {\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  ret 42, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Bar"));
}

TEST_F(CodeGeneratorTest, ReturnErrorEntryNone) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() { Bar(); }\n"
      "  static void Bar() {}\n"
      "}\n");
  EXPECT_EQ("CodeGenerator.Return.None(28) Foo\n", Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, ReturnErrorNone) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(bool x) { if (x) return 1; }\n"
      "}\n");
  EXPECT_EQ("CodeGenerator.Return.None(28) Foo\n", Generate("Sample.Foo"));
}

// variable reference
TEST_F(CodeGeneratorTest, Variable) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() { var x = Bar(); return x; }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  int32 %r4 = call `Sample.Bar`, void\n"
      "  ret %r4, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, While) {
  Prepare(
      "class Sample {\n"
      "  static void Foo(bool x) {\n"
      "    while (Baz()) {\n"
      "      if (x) break;\n"
      "      if (Bar()) continue;\n"
      "    }\n"
      "  }\n"
      "  static bool Bar() { return true; }\n"
      "  static bool Baz() { return true; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 void(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4\n"
      "  bool %b2 = entry\n"
      "  br block4\n"
      "block5:\n"
      "  // In: block9 block4\n"
      "  // Out: block7 block6\n"
      "  br %b2, block7, block6\n"
      "block7:\n"
      "  // In: block5\n"
      "  // Out: block3\n"
      "  br block3\n"
      "block6:\n"
      "  // In: block5\n"
      "  // Out: block9 block8\n"
      "  bool %b8 = call `Sample.Bar`, void\n"
      "  br %b8, block9, block8\n"
      "block9:\n"
      "  // In: block6\n"
      "  // Out: block5\n"
      "  br block5\n"
      "block8:\n"
      "  // In: block6\n"
      "  // Out: block4\n"
      "  br block4\n"
      "block4:\n"
      "  // In: block1 block8\n"
      "  // Out: block5 block3\n"
      "  bool %b11 = call `Sample.Baz`, void\n"
      "  br %b11, block5, block3\n"
      "block3:\n"
      "  // In: block7 block4\n"
      "  // Out: block2\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      Generate("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
