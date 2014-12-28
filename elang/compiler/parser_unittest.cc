// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/test_driver.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// Class
//
TEST(ParserTest, ClassBasic) {
  TestDriver driver("class A : C {} class B : A {} class C {}");
  EXPECT_EQ(
      "class A : C {\n}\n"
      "class B : A {\n}\n"
      "class C {\n}\n",
      driver.RunParser());
}

TEST(ParserTest, ClassField) {
  TestDriver driver("class A { int x; B y = null; var z = 0; }");
  EXPECT_EQ(
      "class A {\n"
      "  int x;\n"
      "  B y = null;\n"
      "  var z = 0;\n"
      "}\n",
      driver.RunParser());
}

TEST(ParserTest, ErrorClassFieldDuplicate) {
  TestDriver driver("class A { int x; bool x; }");
  EXPECT_EQ("Syntax.ClassMember.Duplicate(22) x\n", driver.RunParser());
}

TEST(ParserTest, ErrorClassFieldVar) {
  TestDriver driver("class A { var x; }");
  EXPECT_EQ("Syntax.ClassMember.VarField(14) x\n", driver.RunParser())
      << "var field must be initialized";
}

//////////////////////////////////////////////////////////////////////
//
// 'do' statement
//
TEST(ParserTest, DoBasic) {
  TestDriver driver(
      "class A { void Run(int x) { do { return; } while (x); } }");
  EXPECT_EQ(
      "class A {\n"
      "  void Run(int x) {\n"
      "    do {\n"
      "      return;\n"
      "    } while (x);\n"
      "  }\n"
      "}\n",
      driver.RunParser());
}

//////////////////////////////////////////////////////////////////////
//
// 'if' statement
//
TEST(ParserTest, IfBasic) {
  TestDriver driver("class A { void Run(int x) { if (x) return x; } }");
  EXPECT_EQ(
      "class A {\n"
      "  void Run(int x) {\n"
      "    if (x)\n"
      "      return x;\n"
      "  }\n"
      "}\n",
      driver.RunParser());
}

TEST(ParserTest, IfBasicElse) {
  TestDriver driver(
      "class A {"
      "  void Run(int x) {"
      "    if (x) {"
      "      return x;"
      "    } else {"
      "      return 3;"
      "    }"
      "  }"
      "}");
  EXPECT_EQ(
      "class A {\n"
      "  void Run(int x) {\n"
      "    if (x) {\n"
      "      return x;\n"
      "    } else {\n"
      "      return 3;\n"
      "    }\n"
      "  }\n"
      "}\n",
      driver.RunParser());
}

//////////////////////////////////////////////////////////////////////
//
// Methods
//
TEST(ParserTest, MethodBasic) {
  TestDriver driver("class A { void Run(int x) { return x; } }");
  EXPECT_EQ(
      "class A {\n"
      "  void Run(int x) {\n"
      "    return x;\n"
      "  }\n"
      "}\n",
      driver.RunParser());
}

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
TEST(ParserTest, NamespaceAlias) {
  TestDriver driver("namespace A { using B = N1.N2; }");
  EXPECT_EQ(
      "namespace A {\n"
      "  using B = N1.N2;\n"
      "}\n",
      driver.RunParser());
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
      "}\n",
      driver.RunParser());
}

TEST(ParserTest, NamespaceNestedShortcut) {
  TestDriver driver("namespace N1.N2 { class A {} }");
  EXPECT_EQ(
      "namespace N1 {\n"
      "  namespace N2 {\n"
      "    class A {\n"
      "    }\n"
      "  }\n"
      "}\n",
      driver.RunParser());
}

//////////////////////////////////////////////////////////////////////
//
// enum
//
TEST(ParserTest, EnumBasic) {
  TestDriver driver("enum Color { Red, Green, Blue }");
  EXPECT_EQ(
      "enum Color {\n"
      "  Red,\n"
      "  Green,\n"
      "  Blue,\n"
      "}\n",
      driver.RunParser());
}

TEST(ParserTest, EnumComma) {
  TestDriver driver("enum Color { Red, Green, Blue, }");
  EXPECT_EQ(
      "enum Color {\n"
      "  Red,\n"
      "  Green,\n"
      "  Blue,\n"
      "}\n",
      driver.RunParser())
      << "Comma following last member";
}

TEST(ParserTest, EnumValue) {
  TestDriver driver("enum Color { Red = 3, Green = Red + 10, Blue }");
  EXPECT_EQ(
      "enum Color {\n"
      "  Red = 3,\n"
      "  Green = Red + 10,\n"
      "  Blue,\n"
      "}\n",
      driver.RunParser());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
