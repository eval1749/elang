// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CHARACTER_STREAM_H_
#define ELANG_COMPILER_CHARACTER_STREAM_H_

#include "base/macros.h"
#include "base/strings/string16.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// CharacterStream
//
class CharacterStream {
 public:
  virtual ~CharacterStream();

  virtual bool IsAtEndOfStream() = 0;
  virtual base::char16 ReadChar() = 0;

 protected:
  CharacterStream();

 private:
  DISALLOW_COPY_AND_ASSIGN(CharacterStream);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CHARACTER_STREAM_H_
