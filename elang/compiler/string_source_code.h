// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_STRING_SOURCE_CODE_H_
#define ELANG_COMPILER_STRING_SOURCE_CODE_H_

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
 public:
  StringSourceCode(const base::string16& name,
                   const base::StringPiece16& source);
  ~StringSourceCode() final;

 private:
  // SourceCode
  CharacterStream* GetStream() final { return stream_.get(); }

  std::unique_ptr<CharacterStream> stream_;

  DISALLOW_COPY_AND_ASSIGN(StringSourceCode);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_STRING_SOURCE_CODE_H_
