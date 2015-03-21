// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>

#include "gtest/gtest.h"

#include "elang/targets/bytes.h"
#include "elang/targets/target_features.h"

namespace elang {
namespace targets {
namespace {

class BytesTest : public ::testing::Test {
 protected:
  BytesTest();
  ~BytesTest() override = default;

  Bytes* bytes() { return &bytes_; }

 private:
  Bytes bytes_;
  std::array<uint8_t, 100> data_;

  DISALLOW_COPY_AND_ASSIGN(BytesTest);
};

BytesTest::BytesTest() : bytes_(data_.data(), static_cast<int>(data_.size())) {
}

// Tests

TEST_F(BytesTest, SetBytes) {
  std::array<uint8_t, 10> data;
  for (auto index = 0; index < data.size(); ++index)
    data[index] = index;
  bytes()->SetBytes(10, data.data(), data.size());
  for (auto index = 0; index < data.size(); ++index) {
    EXPECT_EQ(data[index], bytes()->bytes()[index + 10]) << "failed at "
                                                         << index;
  }
}

TEST_F(BytesTest, SetInt32) {
  bytes()->SetInt32(10, 0x11223344);
#if ELANG_TARGET_LITTLE_ENDIAN
  EXPECT_EQ(bytes()->bytes()[10], 0x44);
  EXPECT_EQ(bytes()->bytes()[11], 0x33);
  EXPECT_EQ(bytes()->bytes()[12], 0x22);
  EXPECT_EQ(bytes()->bytes()[13], 0x11);
#else
  EXPECT_EQ(bytes()->bytes()[10], 0x11);
  EXPECT_EQ(bytes()->bytes()[11], 0x22);
  EXPECT_EQ(bytes()->bytes()[12], 0x33);
  EXPECT_EQ(bytes()->bytes()[13], 0x44);
#endif
}

}  // namespace
}  // namespace targets
}  // namespace elang
