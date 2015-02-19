// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/lir/testing/lir_test.h"
#include "elang/lir/value.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirValueTest offers HIR factories.
//
class LirValueTest : public testing::LirTest {
 protected:
  LirValueTest() = default;
  ~LirValueTest() = default;

  static std::string ToString(Value value);

 private:
  DISALLOW_COPY_AND_ASSIGN(LirValueTest);
};

std::string LirValueTest::ToString(Value value) {
  std::stringstream ostream;
  ostream << PrintAsGeneric(value);
  return ostream.str();
}

// Test cases...

TEST_F(LirValueTest, Basic) {
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

TEST_F(LirValueTest, SpillSlot) {
  EXPECT_EQ("$i42", ToString(Value::SpillSlot(Value::Int32Type(), 42)));
  EXPECT_EQ("$i39l", ToString(Value::SpillSlot(Value::Int64Type(), 39)));
  EXPECT_EQ("$f42", ToString(Value::SpillSlot(Value::Float32Type(), 42)));
  EXPECT_EQ("$f39l", ToString(Value::SpillSlot(Value::Float64Type(), 39)));
}

}  // namespace lir
}  // namespace elang
