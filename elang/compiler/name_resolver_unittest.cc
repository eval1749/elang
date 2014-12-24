// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "elang/compiler/testing/test_driver.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {
namespace {

TEST(NameResolverTest, ClassBasic) {
  TestDriver driver(
    "class A : C {} class B : A {} class C {}");
  EXPECT_TRUE(driver.RunNameResolver());
  auto const class_a = driver.FindClass("A");
  EXPECT_TRUE(class_a);
}

}  // namespace
}  // namespace compiler
}  // namespace elang
