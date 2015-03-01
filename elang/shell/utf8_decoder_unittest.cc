// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/shell/utf8_decoder.h"

#include "gtest/gtest.h"

namespace elang {
namespace compiler {
namespace shell {
namespace {

TEST(Utf8DecoderTest, Basic) {
  Utf8Decoder decoder;
  ASSERT_TRUE(decoder.IsValid());
  ASSERT_FALSE(decoder.HasChar());

  decoder.Feed(0x61);
  ASSERT_TRUE(decoder.HasChar());
  EXPECT_EQ(0x61, decoder.Get());

  decoder.Feed(0x78);
  ASSERT_TRUE(decoder.HasChar());
  EXPECT_EQ(0x78, decoder.Get());

  // 0xE6 0x84 0x9B => U+611B
  for (auto const byte : std::vector<uint8_t>{0xE6, 0x84, 0x9B}) {
    ASSERT_FALSE(decoder.HasChar()) << " got char " << decoder.Get();
    decoder.Feed(byte);
  }
  ASSERT_TRUE(decoder.HasChar());
  EXPECT_EQ(0x611B, decoder.Get());

  // 0xF0, 0xA0, 0xAE, 0xB7 => U+20BB7 == U+D842, U+DFB7
  for (auto const byte : std::vector<uint8_t>{0xF0, 0xA0, 0xAE, 0xB7}) {
    ASSERT_FALSE(decoder.HasChar());
    decoder.Feed(byte);
  }
  ASSERT_TRUE(decoder.HasChar());
  EXPECT_EQ(0xD842, decoder.Get());
  EXPECT_EQ(0xDFB7, decoder.Get());
}

}  // namespace
}  // namespace shell
}  // namespace compiler
}  // namespace elang
