// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/compiler/string_stream.h"

#include "base/logging.h"

namespace elang {
namespace compiler {

StringStream::StringStream(const base::StringPiece16& string)
    : position_(0u), string_(string.as_string()) {
}

StringStream::~StringStream() {
}

// CharacterStream
bool StringStream::IsAtEndOfStream() {
  return position_ == string_.length();
}

base::char16 StringStream::ReadChar() {
  DCHECK(!IsAtEndOfStream());
  auto const char_code = string_[position_];
  ++position_;
  return char_code;
}

}  // namespace compiler
}  // namespace elang
