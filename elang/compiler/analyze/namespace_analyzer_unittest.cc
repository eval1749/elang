// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/compiler_test.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzerTest
//
class NamespaceAnalyzerTest : public testing::CompilerTest {
 protected:
  NamespaceAnalyzerTest() = default;
  ~NamespaceAnalyzerTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(NamespaceAnalyzerTest);
};

//////////////////////////////////////////////////////////////////////
//
// Alias resolution
//
TEST_F(NamespaceAnalyzerTest, AliasBasic) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { using C = N1.N2.A; class B : C {} }");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.N2.A", GetBaseClasses("N3.B"));
}

// Same as |AliasBasic|, but order of declaration is different.
TEST_F(NamespaceAnalyzerTest, AliasLayout) {
  Prepare(
      "namespace N3 { using C = N1.N2.A; class B : C {} }"
      "namespace N1.N2 { class A {} }");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.N2.A", GetBaseClasses("N3.B"));
}

TEST_F(NamespaceAnalyzerTest, AliasExtent) {
  Prepare(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class B : R.A {} }"
      "namespace N3 { class C : R.A {} }");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.N2.A", GetBaseClasses("N3.B"));
  EXPECT_EQ("N1.N2.A", GetBaseClasses("N3.C"));
}

TEST_F(NamespaceAnalyzerTest, AliasToAlias) {
  Prepare(
      "using R1 = A.B;"
      "class A { class B { class C {} } }"
      "namespace N1 {"
      "  using R2 = R1;"
      "  class D : R2.C {}"
      "}");
  ASSERT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("A.B.C", GetBaseClasses("N1.D"));
}

TEST_F(NamespaceAnalyzerTest, AliasToAliasDeep) {
  Prepare(
      "using R1 = N1.N2.A.B;"
      "namespace N1 {"
      "  using R2 = R1;"
      "  class D : R2.C {}"
      "  namespace N2 {"
      "     class A { class B { class C {} } }"
      "  }"
      "}");
  ASSERT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.N2.A.B.C", GetBaseClasses("N1.D"));
}

// Note: MS C# compiler doesn't report error if alias A isn't used.
TEST_F(NamespaceAnalyzerTest, AliasErrorAmbiguous) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class A {} }"
      "namespace N3 {"
      "  using A = N1.N2.A;"
      "  class B : A {}"  // A can be N1.N2.A or N3.A.
      "}");
  EXPECT_EQ("NameResolution.Name.Ambiguous(103) A\n",
            AnalyzeNamespace());
}

// Scope of using alias directive is limited into namespace body.
TEST_F(NamespaceAnalyzerTest, AliasErrorScope) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { using R = N1.N2; }"
      "namespace N3 { class B : R.A {} }");  // Error: R unknown
  EXPECT_EQ("NameResolution.Name.NotResolved(88) R\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, AliasErrorScopeHide) {
  Prepare(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  class R {}"        // Class R hides alias R in toplevel.
      "  class B : R.A {}"  // Error: R has no member A.
      "}");
  EXPECT_EQ("NameResolution.Name.NotResolved(86) A\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, AliasErrorScopeResolution) {
  Prepare(
      "namespace N1.N2 {}"
      "namespace N3 {"
      "  using R1 = N1;"
      "  using R2 = N1.N2;"
      "  using R3 = R1.N2;"  // Error: R1 is unknown.
      "}");
  EXPECT_EQ("NameResolution.Name.NotResolved(80) R1\n",
            AnalyzeNamespace());
}

//////////////////////////////////////////////////////////////////////
//
// Class resolution
//
TEST_F(NamespaceAnalyzerTest, ClassBasic) {
  Prepare("class A : C {} class B : A {} class C {}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("C", GetBaseClasses("A"));
  EXPECT_EQ("A", GetBaseClasses("B"));
}

TEST_F(NamespaceAnalyzerTest, ClassNested) {
  Prepare("class A { class B {} }");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("System.Object", GetBaseClasses("A"));
  EXPECT_EQ("System.Object", GetBaseClasses("A.B"));
}

TEST_F(NamespaceAnalyzerTest, ClassErrorBaseNotInterface) {
  Prepare(
      "class A : B, C {}"  // C must be an interface.
      "class B {}"
      "class C {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(13) C\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorBaseClassIsInterface) {
  Prepare(
      "class A : B, C {}"
      "interface B {}"
      "class C {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(13) C\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorBaseClassIsStruct) {
  Prepare(
      "class A : B {}"
      "struct B {}");
  EXPECT_EQ("NameResolution.Name.NeitherClassNorInterface(10) B\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorBaseClassIsNamespace) {
  Prepare("namespace N1 { class A : N1 {} }");
  EXPECT_EQ("NameResolution.Name.NeitherClassNorInterface(25) N1\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorCircularlyDependency) {
  Prepare(
      "class A : B {}"
      "class B : C {}"
      "class C : A {}");
  EXPECT_EQ(
      "NameResolution.Name.Cycle(6) A C\n"
      "NameResolution.Name.Cycle(20) B A\n"
      "NameResolution.Name.Cycle(34) C B\n",
      AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorCircularlyDependencyNested) {
  Prepare(
      "class A : B.C {}"     // A depends on B and C.
      "class B : A {"        // B depends on A.
      "  public class C {}"  // C depends on B.
      "}");
  EXPECT_EQ(
      "NameResolution.Name.Cycle(6) A B\n"
      "NameResolution.Name.Cycle(22) B C\n"
      "NameResolution.Name.Cycle(44) C A\n",
      AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorNestedDependency) {
  Prepare("class A { class B : A {} }");
  EXPECT_EQ("NameResolution.Class.Containing(20) A B\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorSelfReference) {
  Prepare("class A : A {}");
  EXPECT_EQ("NameResolution.Name.Cycle(6) A A\n",
            AnalyzeNamespace());
}

//////////////////////////////////////////////////////////////////////
//
// Predefined types
//
TEST_F(NamespaceAnalyzerTest, PredefinedTypes) {
  Prepare("class A {}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("", GetBaseClasses("System.Object"));
  EXPECT_EQ("System.Object", GetBaseClasses("System.Value"));
  EXPECT_EQ("System.Value", GetBaseClasses("System.Bool"));
  EXPECT_EQ("System.Value", GetBaseClasses("System.Void"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
