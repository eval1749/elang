// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/analyzer_test.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzerTest
//
class NamespaceAnalyzerTest : public testing::AnalyzerTest {
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
  EXPECT_EQ("N1.N2.A System.Object", GetBaseClasses("N3.B"));
  EXPECT_EQ("N1.N2.A", GetDirectBaseClasses("N3.B"));
}

TEST_F(NamespaceAnalyzerTest, AliasConfusing) {
  Prepare(
      "namespace N1 {"
      "  class A {}"
      "  namespace N2 {"
      "    using R1 = A;"  // R1 = N1.A
      "    class A {}"
      "    class B : R1 {}"  // base_class_of(B) = N1.A
      "  }"
      "}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.A", GetDirectBaseClasses("N1.N2.B"));
}

// Same as |AliasBasic|, but order of declaration is different.
TEST_F(NamespaceAnalyzerTest, AliasLayout) {
  Prepare(
      "namespace N3 { using C = N1.N2.A; class B : C {} }"
      "namespace N1.N2 { class A {} }");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.N2.A", GetDirectBaseClasses("N3.B"));
}

TEST_F(NamespaceAnalyzerTest, AliasExtent) {
  Prepare(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class B : R.A {} }"
      "namespace N3 { class C : R.A {} }");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.N2.A", GetDirectBaseClasses("N3.B"));
  EXPECT_EQ("N1.N2.A", GetDirectBaseClasses("N3.C"));
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
  EXPECT_EQ("A.B.C", GetDirectBaseClasses("N1.D"));
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
  EXPECT_EQ("N1.N2.A.B.C", GetDirectBaseClasses("N1.D"));
}

TEST_F(NamespaceAnalyzerTest, AliasErrorAlreadyExists) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class A {} }"
      "namespace N3 { using A = N1.N2.A; }");
  EXPECT_EQ("NameResolution.Alias.Duplicate(78) A A\n", AnalyzeNamespace())
    << "Alias name must be unique in namespace.";
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
  EXPECT_EQ("NameResolution.Alias.Duplicate(79) A A\n", AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, AliasErrorNotFound) {
  Prepare(
      "namespace N {"
      "  using R1 = Foo;"  // |Foo| isn't defined anywhere.
      "  class A : R1 {}"
      "}");
  EXPECT_EQ("NameResolution.Name.NotFound(26) Foo\n", AnalyzeNamespace())
      << "Alias references non-existing thing Foo.";
}

// Scope of using alias directive is limited into namespace body.
TEST_F(NamespaceAnalyzerTest, AliasErrorScope) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { using R = N1.N2; }"
      "namespace N3 { class B : R.A {} }");  // Error: R unknown
  EXPECT_EQ("NameResolution.Name.NotResolved(88) R\n", AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, AliasErrorScopeHide) {
  Prepare(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  class R {}"        // Class R hides alias R in toplevel.
      "  class B : R.A {}"  // Error: R has no member A.
      "}");
  EXPECT_EQ("NameResolution.Name.NotResolved(86) A\n", AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, AliasErrorScopeResolution) {
  Prepare(
      "namespace N1.N2 {}"
      "namespace N3 {"
      "  using R1 = N1;"
      "  using R2 = N1.N2;"
      "  using R3 = R1.N2;"  // Error: R1 is unknown.
      "}");
  EXPECT_EQ("NameResolution.Name.NotResolved(80) R1\n", AnalyzeNamespace());
}

//////////////////////////////////////////////////////////////////////
//
// Class resolution
//
TEST_F(NamespaceAnalyzerTest, ClassBasic) {
  Prepare("class A : C {} class B : A {} class C {}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("C", GetDirectBaseClasses("A"));
  EXPECT_EQ("A", GetDirectBaseClasses("B"));
}

TEST_F(NamespaceAnalyzerTest, ClassNested) {
  Prepare("class A { class B {} }");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("System.Object", GetDirectBaseClasses("A"));
  EXPECT_EQ("System.Object", GetDirectBaseClasses("A.B"));
}

TEST_F(NamespaceAnalyzerTest, ClassErrorBaseNotInterface) {
  Prepare(
      "class A : B, C {}"  // C must be an interface.
      "class B {}"
      "class C {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(13) C\n", AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorBaseStruct) {
  Prepare(
      "class A : S {}"
      "struct S {}");
  EXPECT_EQ("NameResolution.Name.NeitherClassNorInterface(10) S\n",
            AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorBaseClassIsInterface) {
  Prepare(
      "class A : B, C {}"
      "interface B {}"
      "class C {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(13) C\n", AnalyzeNamespace());
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

TEST_F(NamespaceAnalyzerTest, ClassErrorDuplicate) {
  Prepare("namespace System { class Int32 {} }");
  // Note: class 'System.Int32' i installed by |NamespaceAnalyzerTest|, before
  // paring.
  EXPECT_EQ("Syntax.Class.Duplicate(25) Int32 Int32\n", Format());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorNestedDependency) {
  Prepare("class A { class B : A {} }");
  EXPECT_EQ("NameResolution.Class.Containing(20) A B\n", AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ClassErrorSelfReference) {
  Prepare("class A : A {}");
  EXPECT_EQ("NameResolution.Name.Cycle(6) A A\n", AnalyzeNamespace());
}

//////////////////////////////////////////////////////////////////////
//
// Import
//
TEST_F(NamespaceAnalyzerTest, ImportBasic) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  using N1.N2;"
      "  class B : A {}"
      "}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.N2.A", GetDirectBaseClasses("N3.B"));
}

TEST_F(NamespaceAnalyzerTest, ImportConfusing) {
  Prepare(
      "namespace N1 { class A {} }"
      "namespace N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  using N1 = N2;"
      "  class B : N1.A {}"
      "}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N2.A", GetDirectBaseClasses("N3.B"));
}

TEST_F(NamespaceAnalyzerTest, ImportErrorAmbiguous) {
  Prepare(
      "namespace N1 { class A {} }"
      "namespace N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  using N2;"
      "  class B : A {}"  // A is ambiguous
      "}");
  EXPECT_EQ("NameResolution.Name.Ambiguous(102) A\n", AnalyzeNamespace());
}

TEST_F(NamespaceAnalyzerTest, ImportErrorNestNamespace) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  class B : N2.A {}"
      "}");
  EXPECT_EQ("NameResolution.Name.NotResolved(67) N2\n", AnalyzeNamespace())
      << "using N1 should not import namespace N1.N2 into N3.";
}

TEST_F(NamespaceAnalyzerTest, ImportNotAmbiguous) {
  Prepare(
      "namespace N1 { class A {} }"
      "namespace N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  using N2;"
      "  using A = N1.A;"
      "  class B : A {}"  // A means N1.A
      "}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("N1.A", GetDirectBaseClasses("N3.B"));
}

//////////////////////////////////////////////////////////////////////
//
// Interface
//
TEST_F(NamespaceAnalyzerTest, InterfaceBasic) {
  Prepare(
      "interface I {}"
      "interface J {}"
      "interface K : I {}"
      "interface L : K, J {}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("K J I", GetBaseClasses("L"));
  EXPECT_EQ("K J", GetDirectBaseClasses("L"));
}

TEST_F(NamespaceAnalyzerTest, InterfaceBaseClasses) {
  // Taken from common-lisp sample
  Prepare(
      "interface A {}"
      "interface B {}"
      "interface C {}"
      "interface S : A, B {}"
      "interface R : A, C {}"
      "interface Q : S, R {}");
  EXPECT_EQ("", AnalyzeNamespace());
  // common-lisp result = "S R A C B"
  EXPECT_EQ("S R A B C", GetBaseClasses("Q"));
}

TEST_F(NamespaceAnalyzerTest, InterfaceBaseClasses2) {
  // Taken from common-lisp sample
  Prepare(
      "interface pie : apple, cinnamon {}"
      "interface apple : fruit {}"
      "interface cinnamon : spice {}"
      "interface fruit : food {}"
      "interface spice  : food {}"
      "interface food {}");
  EXPECT_EQ("", AnalyzeNamespace());
  // common-lisp result = "apple fruit cinnamon spice food"
  EXPECT_EQ("apple cinnamon fruit spice food", GetBaseClasses("pie"));
}

TEST_F(NamespaceAnalyzerTest, InterfaceErrorBaseClass) {
  Prepare(
      "class A {}"
      "interface I : A {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(24) A\n", AnalyzeNamespace());
}

//////////////////////////////////////////////////////////////////////
//
// Predefined types
//
TEST_F(NamespaceAnalyzerTest, PredefinedTypes) {
  Prepare("class A {}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("", GetDirectBaseClasses("System.Object"));
  EXPECT_EQ("System.Object", GetDirectBaseClasses("System.ValueType"));
  EXPECT_EQ("System.ValueType", GetDirectBaseClasses("System.Bool"));
  EXPECT_EQ("System.ValueType", GetDirectBaseClasses("System.Void"));
}

//////////////////////////////////////////////////////////////////////
//
// Struct
//
TEST_F(NamespaceAnalyzerTest, StructBasic) {
  Prepare(
      "interface I {}"
      "interface J {}"
      "struct S : I, J {}");
  EXPECT_EQ("", AnalyzeNamespace());
  EXPECT_EQ("System.ValueType I J System.Object", GetBaseClasses("S"));
  EXPECT_EQ("System.ValueType I J", GetDirectBaseClasses("S"));
}

TEST_F(NamespaceAnalyzerTest, StructErrorBaseClass) {
  Prepare(
      "class A {}"
      "struct S : A {}");
  EXPECT_EQ("NameResolution.Name.NotInterface(21) A\n", AnalyzeNamespace());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
