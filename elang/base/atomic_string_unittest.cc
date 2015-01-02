// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/atomic_string.h"
#include "elang/base/atomic_string_factory.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

TEST(AtomicStringTest, NewAtomicString) {
  AtomicStringFactory factory;
  auto const name1 = factory.NewAtomicString(L"foo");
  auto const name2 = factory.NewAtomicString(L"foo");
  EXPECT_EQ(name1, name2);
  EXPECT_EQ(L"foo", name1->string().as_string());
}

TEST(AtomicStringTest, NewUniqueAtomicString) {
  AtomicStringFactory factory;
  auto const name1 = factory.NewUniqueAtomicString(L"foo%d");
  auto const name2 = factory.NewUniqueAtomicString(L"foo%d");
  EXPECT_NE(name1, name2);
}

}  // namespace
}  // namespace elang
