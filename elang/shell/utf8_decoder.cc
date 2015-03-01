// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/shell/utf8_decoder.h"

#include "base/logging.h"

namespace elang {
namespace shell {

//////////////////////////////////////////////////////////////////////
//
// Utf8Decoder
//
Utf8Decoder::Utf8Decoder() : char32_(-1), need_bytes_(0) {
}

void Utf8Decoder::BadFeed() {
  need_bytes_ = -1;
}

// 1 U+0000   U+007F   0xxxxxxx
// 2 U+0080   U+07FF   110xxxxx 10xxxxxx
// 3 U+0800   U+FFFF   1110xxxx 10xxxxxx 10xxxxxx
// 4 U+10000  U+1FFFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// Note: We don't support 5 byte and 6 byte UTF-8 sequence yet.
void Utf8Decoder::Feed(uint8_t byte) {
  DCHECK(IsValid());
  if (char32_ >= 0) {
    DCHECK_GE(need_bytes_, 1);
    if (byte < 0x80 || byte > 0xBF) {
      BadFeed();
      return;
    }
    char32_ <<= 6;
    char32_ |= byte & 0x3F;
    --need_bytes_;
    if (char32_ > 0x10FFFF)
      BadFeed();
    return;
  }

  DCHECK(!need_bytes_);
  DCHECK_EQ(char32_, -1);
  if (byte <= 0x7E) {
    char32_ = byte;
    need_bytes_ = 0;
    return;
  }
  if (byte >= 0xC0 && byte <= 0xDF) {
    char32_ = byte & 0x1F;
    need_bytes_ = 1;
    return;
  }
  if (byte >= 0xE0 && byte <= 0xEF) {
    char32_ = byte & 0x0F;
    need_bytes_ = 2;
    return;
  }
  if (byte >= 0xF0 && byte <= 0xF4) {
    char32_ = byte & 7;
    need_bytes_ = 3;
    return;
  }
  BadFeed();
}

int Utf8Decoder::Get() {
  DCHECK(!need_bytes_);
  DCHECK_GE(char32_, 0);
  if (char32_ <= 0xFFFF) {
    auto const result = char32_;
    char32_ = -1;
    return result;
  }
  DCHECK_LE(char32_, 0x10FFFF);
  char32_ -= 0x10000;
  auto const result = 0xD800 | ((char32_ >> 10) & 0x3FF);
  char32_ = 0xDC00 | (char32_ & 0x3FF);
  return result;
}

}  // namespace shell
}  // namespace elang
