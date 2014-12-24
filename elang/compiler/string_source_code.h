// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_string_code_h)
#define INCLUDE_elang_compiler_string_code_h

#include <memory>

#include "base/strings/string_piece.h"
#include "elang/compiler/source_code.h"

namespace elang {
namespace compiler {

class CharacterStream;

//////////////////////////////////////////////////////////////////////
//
// StringSourceCode
//
class StringSourceCode final : public SourceCode {
  private: std::unique_ptr<CharacterStream> stream_;

  public: StringSourceCode(const base::string16& name,
                           const base::StringPiece16& source);
  public: ~StringSourceCode() final;

  // SourceCode
  private: CharacterStream* GetStream() final { return stream_.get(); }

  DISALLOW_COPY_AND_ASSIGN(StringSourceCode);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_string_code_h)

