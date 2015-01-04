// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/value.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {

TEST(LirValueTest, Basic) {
  EXPECT_EQ(Value(), Value());
  EXPECT_EQ(Value(Value::Kind::Immediate, 3), Value(Value::Kind::Immediate, 3));
  EXPECT_NE(Value(Value::Kind::Immediate, 4), Value(Value::Kind::Immediate, 3));
}

}  // namespace lir
}  // namespace elang
