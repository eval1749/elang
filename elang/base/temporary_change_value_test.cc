// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/temporary_change_value.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

TEST(TemporaryChangeValueTest, Basic) {
  int value = 123;
  {
    TemporaryChangeValue<int> scope(value, 42);
    EXPECT_EQ(42, value);
  }
  EXPECT_EQ(123, value);
}

}  // namespace
}  // namespace elang
