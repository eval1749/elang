// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_UTF8_DECODER_H_
#define ELANG_SHELL_UTF8_DECODER_H_

#include <vector>

#include "base/basictypes.h"

namespace elang {
namespace compiler {
namespace shell {

//////////////////////////////////////////////////////////////////////
//
// Utf8Decoder
//
class Utf8Decoder final {
 public:
  Utf8Decoder();
  ~Utf8Decoder() = default;

  void Feed(uint8_t byte);
  int Get();
  bool HasChar() const { return char32_ >= 0 && !need_bytes_; }
  bool IsValid() const { return need_bytes_ >= 0; }

 private:
  void BadFeed();

  int char32_;
  int need_bytes_;

  DISALLOW_COPY_AND_ASSIGN(Utf8Decoder);
};

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_UTF8_DECODER_H_
