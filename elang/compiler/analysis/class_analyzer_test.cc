// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/analyzer_test.h"

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

//////////////////////////////////////////////////////////////////////
//
// Method resolution
//
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
      "System.Bool Sample.Foo(System.Int32 x)\n"
      "System.Bool Sample.Foo(System.Float32 x)\n"
      "System.Bool Sample.Foo(System.Float64 x)\n"
      "System.Char Sample.Foo(System.Char x, System.Int32 y)\n",
      GetMethodGroup("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
