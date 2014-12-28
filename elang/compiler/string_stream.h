// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_string_stream_h)
#define INCLUDE_elang_compiler_string_stream_h

#include "base/strings/string_piece.h"
#include "elang/compiler/character_stream.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// StringStream
//
class StringStream final : public CharacterStream {
 public:
  explicit StringStream(const base::StringPiece16& source);
  ~StringStream() final;

  // CharacterStream
 private:
  bool IsAtEndOfStream() final;
  base::char16 ReadChar() final;

  const base::string16 string_;
  size_t position_;

  DISALLOW_COPY_AND_ASSIGN(StringStream);
};

}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_string_stream_h)

