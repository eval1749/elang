// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/analyzer_test.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// MethodAnalyzerTest
//
class MethodAnalyzerTest : public testing::AnalyzerTest {
 protected:
  MethodAnalyzerTest() = default;
  ~MethodAnalyzerTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(MethodAnalyzerTest);
};

//////////////////////////////////////////////////////////////////////
//
// Method resolution
//
TEST_F(MethodAnalyzerTest, Method) {
  Prepare(
      "class Sample {"
      "    void Main() { ConsoleWrite(\"Hello world!\"); }"
      "  }");
  EXPECT_EQ("", AnalyzeClass());
  EXPECT_EQ("(method Main (signature (class Void) ()))\n",
            GetMethodGroup("Sample.Main"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
