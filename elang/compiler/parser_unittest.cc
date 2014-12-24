// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "elang/compiler/testing/test_driver.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {
namespace {

TEST(ParserTest, ClassBasic) {
  TestDriver driver(
    "class A : C {} class B : A {} class C {}");
  EXPECT_EQ(
    "class A : C {\n}\n"
    "class B : A {\n}\n"
    "class C {\n}\n", driver.RunParser());
}

TEST(ParserTest, NamespaceAlias) {
  TestDriver driver(
    "namespace A { using B = N1.N2; }");
  EXPECT_EQ(
    "namespace A {\n"
    "  using B = N1.N2;\n"
    "}\n", driver.RunParser());
}

TEST(ParserTest, NamespaceBasic) {
  TestDriver driver(
    "namespace A { namespace B { namespace C {} } }\n"
    "namespace D {}");
  EXPECT_EQ(
    "namespace A {\n"
    "  namespace B {\n"
    "    namespace C {\n"
    "    }\n"
    "  }\n"
    "}\n"
    "namespace D {\n"
    "}\n", driver.RunParser());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
