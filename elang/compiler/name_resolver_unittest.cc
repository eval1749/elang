// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/test_driver.h"
#include "elang/hir/class.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {
namespace {

TEST(NameResolverTest, ClassBasic) {
  TestDriver driver(
    "class A : C {} class B : A {} class C {}");
  EXPECT_EQ("", driver.RunNameResolver());

  auto const class_a = driver.FindClass("A");
  EXPECT_TRUE(class_a) << "Not found: class A";

  auto const class_b = driver.FindClass("B");
  EXPECT_TRUE(class_b) << "Not found: class B";

  auto const class_c = driver.FindClass("C");
  EXPECT_TRUE(class_c) << "Not found: class C";

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
  EXPECT_EQ("NameResolution.Name.Cyclic(20) B A\n"
            "NameResolution.Name.Cyclic(34) C B\n"
            "NameResolution.Name.Cyclic(6) A C\n",
            driver.RunNameResolver());
}

TEST(NameResolverTest, ErrorClassCircularlyDependencyNested) {
  TestDriver driver(
    "class A : B.C {}"
    "class B { public class C {} }");
  EXPECT_EQ("NameResolution.Name.NeitherNamespaceOrType(12) C\n",
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
  EXPECT_EQ("NameResolution.Name.Cyclic(6) A A\n", driver.RunNameResolver());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
