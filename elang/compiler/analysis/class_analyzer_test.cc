// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/analyzer_test.h"

#include "elang/compiler/ast/nodes.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzerTest
//
class ClassAnalyzerTest : public testing::AnalyzerTest {
 protected:
  ClassAnalyzerTest() = default;
  ~ClassAnalyzerTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ClassAnalyzerTest);
};

TEST_F(ClassAnalyzerTest, Enum) {
  Prepare("enum Color { Red, Green, Blue }");
  EXPECT_EQ("", AnalyzeClass());
  auto const node = FindMember("Color");
  auto const semantic = SemanticOf(node);
  EXPECT_EQ("enum Color : System.Int32 {Red = 0, Green = 1, Blue = 2}",
            ToString(semantic));
}

TEST_F(ClassAnalyzerTest, EnumConstExpr) {
  Prepare("enum Color { Red = 1, Green = Red + 2, Blue = Red + 4 }");
  EXPECT_EQ("", AnalyzeClass());
  EXPECT_EQ("Color.Red = 1", ToString(SemanticOf(FindMember("Color.Red"))));
  EXPECT_EQ("Color.Green = 3", ToString(SemanticOf(FindMember("Color.Green"))));
  EXPECT_EQ("Color.Blue = 5", ToString(SemanticOf(FindMember("Color.Blue"))));
}

TEST_F(ClassAnalyzerTest, EnumConstExprForwardReference) {
  Prepare("enum Color { Red = Blue, Green = Blue + 2, Blue = 1}");
  EXPECT_EQ("", AnalyzeClass());
  EXPECT_EQ("Color.Red = 1", ToString(SemanticOf(FindMember("Color.Red"))));
  EXPECT_EQ("Color.Green = 3", ToString(SemanticOf(FindMember("Color.Green"))));
  EXPECT_EQ("Color.Blue = 1", ToString(SemanticOf(FindMember("Color.Blue"))));
}

TEST_F(ClassAnalyzerTest, EnumConstExprWithAnotherEnum) {
  Prepare("enum E1 { M = E2.N } enum E2 { N = 42 }");
  EXPECT_EQ("", AnalyzeClass());
  EXPECT_EQ("E1.M = 42", ToString(SemanticOf(FindMember("E1.M"))));
  EXPECT_EQ("E2.N = 42", ToString(SemanticOf(FindMember("E2.N"))));
}

TEST_F(ClassAnalyzerTest, EnumErrorCycle) {
  Prepare("enum Color { Red = Green, Green = Blue, Blue = Red }");
  EXPECT_EQ(
      "Analyze.Expression.Cycle(13) Red Green\n"
      "Analyze.Expression.Cycle(26) Green Blue\n"
      "Analyze.Expression.Cycle(40) Blue Red\n",
      AnalyzeClass());
}

TEST_F(ClassAnalyzerTest, EnumErrorCycleSelf) {
  Prepare("enum Color { Red = Red }");
  EXPECT_EQ("Analyze.Expression.Cycle(13) Red Red\n", AnalyzeClass());
}

TEST_F(ClassAnalyzerTest, EnumErrorCycleWithAnotherEnum) {
  Prepare("enum E1 { M = E2.N } enum E2 { N = E1.M }");
  EXPECT_EQ(
      "Analyze.Expression.Cycle(10) M E2.N\n"
      "Analyze.Expression.Cycle(31) N E1.M\n",
      AnalyzeClass());
}

TEST_F(ClassAnalyzerTest, EnumErrorNotInt) {
  Prepare("enum Color { Red = 'C', Green = 1.0, Blue = \"str\" }");
  EXPECT_EQ(
      "Analyze.Expression.Type(19) 'C' Int32\n"
      "Analyze.Expression.Type(32) 1 Int32\n"
      "Analyze.Expression.Type(44) \"str\" Int32\n",
      AnalyzeClass());
}

TEST_F(ClassAnalyzerTest, EnumErrorOverflow) {
  Prepare("enum Color : int8 { Red = 127, Green, Blue }");
  EXPECT_EQ("Semantic.Value.Type(31) 128 Int8\n", AnalyzeClass());
}

TEST_F(ClassAnalyzerTest, Method) {
  Prepare(
      "class Sample {"
      "    bool Foo(int x) { return x > 10; }"
      "    bool Foo(float32 x) { return x > 10; }"
      "    bool Foo(float64 x) { return x > 10; }"
      "    char Foo(char x, int y) { return x + y; }"
      "  }");
  EXPECT_EQ("", AnalyzeClass());
  EXPECT_EQ(
      "System.Bool Sample.Foo(System.Int32)\n"
      "System.Bool Sample.Foo(System.Float32)\n"
      "System.Bool Sample.Foo(System.Float64)\n"
      "System.Char Sample.Foo(System.Char, System.Int32)\n",
      GetMethodGroup("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
