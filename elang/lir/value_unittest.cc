// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/value.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {

TEST(LirValueTest, Basic) {
  EXPECT_EQ(Value(), Value());
  auto const value1 = Value(Value::Type::Integer, ValueSize::Size32,
                            Value::Kind::Immediate, 42);
  auto const value2 = Value(Value::Type::Integer, ValueSize::Size32,
                            Value::Kind::Immediate, 42);
  auto const value3 = Value(Value::Type::Integer, ValueSize::Size32,
                            Value::Kind::Immediate, 123);
  EXPECT_EQ(value1, value2);
  EXPECT_NE(value1, value3);
}

}  // namespace lir
}  // namespace elang
