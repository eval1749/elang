// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(elang_compiler_character_stream_h)
#define elang_compiler_character_stream_h

#include "base/macros.h"
#include "base/strings/string16.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// CharacterStream
//
class CharacterStream {

  protected: CharacterStream();
  public: virtual ~CharacterStream();

  public: virtual bool IsAtEndOfStream() = 0;
  public: virtual base::char16 ReadChar() = 0;

  DISALLOW_COPY_AND_ASSIGN(CharacterStream);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(elang_compiler_character_stream_h)

