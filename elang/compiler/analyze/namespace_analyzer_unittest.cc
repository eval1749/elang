// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/class.h"
#include "elang/compiler/testing/test_driver.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {
namespace {

using testing::TestDriver;

//////////////////////////////////////////////////////////////////////
//
// Alias resolution
//
TEST(NamespaceAnalyzerTest, AliasBasic) {
  TestDriver driver(
      "namespace N1.N2 { class A{} }"
      "namespace N3 { using C = N1.N2.A; class B : C {} }");
  EXPECT_EQ("", driver.RunNamespaceAnalyzer());
  EXPECT_EQ("N1.N2.A", driver.GetBaseClasses("N3.B"));
}

// Same as |AliasBasic|, but order of declaration is different.
TEST(NamespaceAnalyzerTest, AliasLayout) {
  TestDriver driver(
      "namespace N3 { using C = N1.N2.A; class B : C {} }"
      "namespace N1.N2 { class A {} }");
  EXPECT_EQ("", driver.RunNamespaceAnalyzer());
  EXPECT_EQ("N1.N2.A", driver.GetBaseClasses("N3.B"));
}

TEST(NamespaceAnalyzerTest, AliasExtent) {
  TestDriver driver(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class B : R.A {} }"
      "namespace N3 { class C : R.A {} }");
  EXPECT_EQ("", driver.RunNamespaceAnalyzer());
  EXPECT_EQ("N1.N2.A", driver.GetBaseClasses("N3.B"));
  EXPECT_EQ("N1.N2.A", driver.GetBaseClasses("N3.C"));
}

TEST(NamespaceAnalyzerTest, AliasToAlias) {
  TestDriver driver(
      "using R1 = A.B;"
      "class A { class B { class C {} } }"
      "namespace N1 {"
      "  using R2 = R1;"
      "  class D : R2.C {}"
      "}");
  ASSERT_EQ("", driver.RunNamespaceAnalyzer());
  EXPECT_EQ("A.B.C", driver.GetBaseClasses("N1.D"));
}

// Note: MS C# compiler doesn't report error if alias A isn't used.
TEST(NamespaceAnalyzerTest, ErrorAliasAmbiguous) {
  TestDriver driver(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class A {} }"
      "namespace N3 {"
      "  using A = N1.N2.A;"
      "  class B : A {}"  // A can be N1.N2.A or N3.A.
      "}");
  EXPECT_EQ("NameResolution.Name.Ambiguous(103) A\n",
            driver.RunNamespaceAnalyzer());
}

// Scope of using alias directive is limited into namespace body.
TEST(NamespaceAnalyzerTest, ErrorAliasScope) {
  TestDriver driver(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { using R = N1.N2; }"
      "namespace N3 { class B : R.A {} }");  // Error: R unknown
  EXPECT_EQ("NameResolution.Name.NotFound(88) R\n",
            driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorAliasScopeHide) {
  TestDriver driver(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  class R {}"        // Class R hides alias R in toplevel.
      "  class B : R.A {}"  // Error: R has no member A.
      "}");
  EXPECT_EQ("NameResolution.Name.NotFound(86) A\n",
            driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorAliasScopeResolution) {
  TestDriver driver(
      "namespace N1.N2 {}"
      "namespace N3 {"
      "  using R1 = N1;"
      "  using R2 = N1.N2;"
      "  using R3 = R1.N2;"  // Error: R1 is unknown.
      "}");
  EXPECT_EQ(
      "NameResolution.Alias.NoTarget(75) R3\n"
      "NameResolution.Name.NotFound(80) R1\n",
      driver.RunNamespaceAnalyzer());
}

//////////////////////////////////////////////////////////////////////
//
// Class resolution
//
TEST(NamespaceAnalyzerTest, ClassBasic) {
  TestDriver driver("class A : C {} class B : A {} class C {}");
  EXPECT_EQ("", driver.RunNamespaceAnalyzer());
  EXPECT_EQ("C", driver.GetBaseClasses("A"));
  EXPECT_EQ("A", driver.GetBaseClasses("B"));
}

TEST(NamespaceAnalyzerTest, ClassNested) {
  TestDriver driver("class A { class B {} }");
  EXPECT_EQ("", driver.RunNamespaceAnalyzer());
  EXPECT_EQ("", driver.GetBaseClasses("A"));
  EXPECT_EQ("", driver.GetBaseClasses("A.B"));
}

TEST(NamespaceAnalyzerTest, ErrorClassBaseNotInterface) {
  TestDriver driver(
      "class A : B, C {}"  // C must be an interface.
      "class B {}"
      "class C {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(13) C\n",
            driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorClassBaseClassIsInterface) {
  TestDriver driver(
      "class A : B, C {}"
      "interface B {}"
      "class C {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(13) C\n",
            driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorClassBaseClassIsStruct) {
  TestDriver driver(
      "class A : B {}"
      "struct B {}");
  EXPECT_EQ("NameResolution.Name.NeitherClassNortInterface(10) B\n",
            driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorClassBaseClassIsNamespace) {
  TestDriver driver("namespace N1 { class A : N1 {} }");
  EXPECT_EQ("NameResolution.Name.NotClass(25) N1\n",
            driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorClassCircularlyDependency) {
  TestDriver driver(
      "class A : B {}"
      "class B : C {}"
      "class C : A {}");
  EXPECT_EQ(
      "NameResolution.Name.Cycle(6) A C\n"
      "NameResolution.Name.Cycle(20) B A\n"
      "NameResolution.Name.Cycle(34) C B\n",
      driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorClassCircularlyDependencyNested) {
  TestDriver driver(
      "class A : B.C {}"     // A depends on B and C.
      "class B : A {"        // B depends on A.
      "  public class C {}"  // C depends on B.
      "}");
  EXPECT_EQ(
      "NameResolution.Name.Cycle(6) A B\n"
      "NameResolution.Name.Cycle(22) B C\n"
      "NameResolution.Name.Cycle(44) C A\n",
      driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorClassNestedDependency) {
  TestDriver driver("class A { class B : A {} }");
  EXPECT_EQ("NameResolution.Class.Containing(20) A B\n",
            driver.RunNamespaceAnalyzer());
}

TEST(NamespaceAnalyzerTest, ErrorClassSelfReference) {
  TestDriver driver("class A : A {}");
  EXPECT_EQ("NameResolution.Name.Cycle(6) A A\n",
            driver.RunNamespaceAnalyzer());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
