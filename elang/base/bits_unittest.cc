// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/bits.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

const auto kOne32 = static_cast<uint32_t>(1);
const auto kOne64 = static_cast<uint64_t>(1);

TEST(Bits, CountLeadingZeros) {
  EXPECT_EQ(2, CountLeadingZeros(kOne32 << 29));
  EXPECT_EQ(2, CountLeadingZeros(kOne64 << 61));
}

TEST(Bits, CountPopulation) {
  EXPECT_EQ(1, CountPopulation(kOne32 << 29));
  EXPECT_EQ(1, CountPopulation(kOne64 << 60));

  EXPECT_EQ(2, CountPopulation(kOne32 * 9));
  EXPECT_EQ(2, CountPopulation(kOne64 * 9));
}

TEST(Bits, CountTrailingZeros) {
  EXPECT_EQ(29, CountTrailingZeros(kOne32 << 29));
  EXPECT_EQ(61, CountTrailingZeros(kOne64 << 61));
}

}  // namespace
}  // namespace elang
