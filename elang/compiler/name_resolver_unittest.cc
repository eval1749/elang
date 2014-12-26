// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/class.h"
#include "elang/compiler/testing/test_driver.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// Alias resolution
//
TEST(NameResolverTest, AliasBasic) {
  TestDriver driver(
    "namespace N1.N2 { class A{} }"
    "namespace N3 { using C = N1.N2.A; class B : C {} }");
  EXPECT_EQ("", driver.RunNameResolver());
  auto const class_a = driver.FindClass("N1.N2.A");
  ASSERT_TRUE(class_a) << "Not found class N1.N2.A";
  auto const class_b = driver.FindClass("N3.B");
  ASSERT_TRUE(class_b) << "Not found class N3.B";
  EXPECT_EQ(class_a, class_b->base_classes()[0]);
}

// Same as |AliasBasic|, but order of declaration is different.
TEST(NameResolverTest, AliasLayout) {
  TestDriver driver(
    "namespace N3 { using C = N1.N2.A; class B : C {} }"
    "namespace N1.N2 { class A {} }");
  EXPECT_EQ("", driver.RunNameResolver());
  auto const class_a = driver.FindClass("N1.N2.A");
  ASSERT_TRUE(class_a) << "Not found class N1.N2.A";
  auto const class_b = driver.FindClass("N3.B");
  ASSERT_TRUE(class_b) << "Not found class N3.B";
  EXPECT_EQ(class_a, class_b->base_classes()[0]);
}

TEST(NameResolverTest, AliasExtent) {
  TestDriver driver(
    "using R = N1.N2;"
    "namespace N1.N2 { class A {} }"
    "namespace N3 { class B : R.A {} }"
    "namespace N3 { class C : R.A {} }");
  EXPECT_EQ("", driver.RunNameResolver());

  auto const class_a = driver.FindClass("N1.N2.A");
  ASSERT_TRUE(class_a) << "Not found class N1.N2.A";
  auto const class_b = driver.FindClass("N3.B");
  ASSERT_TRUE(class_b) << "Not found class N3.B";
  auto const class_c = driver.FindClass("N3.C");
  ASSERT_TRUE(class_c) << "Not found class N3.C";
  EXPECT_EQ(class_a, class_b->base_classes()[0]);
  EXPECT_EQ(class_a, class_c->base_classes()[0]);
}

// Note: MS C# compiler doesn't report error if alias A isn't used.
TEST(NameResolverTest, ErrorAliasAmbiguous) {
  TestDriver driver(
    "namespace N1.N2 { class A {} }"
    "namespace N3 { class A {} }"
    "namespace N3 {"
    "  using A = N1.N2.A;"
    "  class B : A {}" // A can be N1.N2.A or N3.A.
    "}");
  EXPECT_EQ("NameResolution.Name.Ambiguous(103) A\n",
            driver.RunNameResolver());
}

// Scope of using alias directive is limited into namespace body.
TEST(NameResolverTest, ErrorAliasScope) {
  TestDriver driver(
    "namespace N1.N2 { class A {} }"
    "namespace N3 { using R = N1.N2; }"
    "namespace N3 { class B : R.A {} }"); // Error: R unknown
  EXPECT_EQ("NameResolution.Name.NotFound(88) R\n",
            driver.RunNameResolver());
}

TEST(NameResolverTest, ErrorAliasScopeHide) {
  TestDriver driver(
    "using R = N1.N2;"
    "namespace N1.N2 { class A {} }"
    "namespace N3 {"
    "  class R {}"  // Class R hides alias R in toplevel.
    "  class B : R.A {}" // Error: R has no member A.
    "}");
  EXPECT_EQ("NameResolution.Name.NotFound(86) A\n", driver.RunNameResolver());
}

TEST(NameResolverTest, ErrorAliasScopeResolution) {
  TestDriver driver(
    "namespace N1.N2 {}"
    "namespace N3 {"
    "  using R1 = N1;"
    "  using R2 = N1.N2;"
    "  using R3 = R1.N2;" // Error: R1 is unknown.
    "}");
  EXPECT_EQ("NameResolution.Name.NotFound(80) R1\n"
            "NameResolution.Alias.NoTarget(75) R3\n",
             driver.RunNameResolver());
}

//////////////////////////////////////////////////////////////////////
//
// Class resolution
//
TEST(NameResolverTest, ClassBasic) {
  TestDriver driver(
    "class A : C {} class B : A {} class C {}");
  EXPECT_EQ("", driver.RunNameResolver());

  auto const class_a = driver.FindClass("A");
  ASSERT_TRUE(class_a) << "Not found: class A";

  auto const class_b = driver.FindClass("B");
  ASSERT_TRUE(class_b) << "Not found: class B";

  auto const class_c = driver.FindClass("C");
  ASSERT_TRUE(class_c) << "Not found: class C";

  // Check class hierarchy
  EXPECT_EQ(class_c, class_a->base_classes()[0]);
  EXPECT_EQ(class_a, class_b->base_classes()[0]);
}

TEST(NameResolverTest, ClassNested) {
  TestDriver driver(
    "class A { class B {} }");
  EXPECT_EQ("", driver.RunNameResolver());

  auto const class_a = driver.FindClass("A");
  EXPECT_TRUE(class_a) << "Not found: class A";

  auto const class_b = driver.FindClass("A.B");
  EXPECT_TRUE(class_b) << "Not found: class A.B";
}

TEST(NameResolverTest, ErrorClassBaseClass) {
  TestDriver driver(
    "namespace N1 { class A : N1 {} }");
  EXPECT_EQ("NameResolution.Name.NotClass(25) N1\n", driver.RunNameResolver());
}

TEST(NameResolverTest, ErrorClassCircularlyDependency) {
  TestDriver driver(
    "class A : B {}"
    "class B : C {}"
    "class C : A {}");
  EXPECT_EQ("NameResolution.Name.Cycle(20) B A\n"
            "NameResolution.Name.Cycle(34) C B\n"
            "NameResolution.Name.Cycle(6) A C\n",
            driver.RunNameResolver());
}

TEST(NameResolverTest, ErrorClassCircularlyDependencyNested) {
  TestDriver driver(
    "class A : B.C {}" // A depends on B and C.
    "class B : A {" // B depends on A.
    "  public class C {}" // C depends on B.
    "}");
  EXPECT_EQ("NameResolution.Name.Cycle(44) C A\n"
            "NameResolution.Name.Cycle(6) A B\n"
            "NameResolution.Name.Cycle(22) B C\n",
            driver.RunNameResolver());
}

TEST(NameResolverTest, ErrorClassNestedDependency) {
  TestDriver driver(
    "class A { class B : A {} }");
  EXPECT_EQ("NameResolution.Class.Containing(20) A B\n",
            driver.RunNameResolver());
}

TEST(NameResolverTest, ErrorClassSelfReference) {
  TestDriver driver(
    "class A : A {}");
  EXPECT_EQ("NameResolution.Name.Cycle(6) A A\n", driver.RunNameResolver());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
