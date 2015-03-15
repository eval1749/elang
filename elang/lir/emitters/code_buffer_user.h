// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_CODE_BUFFER_USER_H_
#define ELANG_LIR_EMITTERS_CODE_BUFFER_USER_H_

#include "base/basictypes.h"
#include "base/strings/string_piece.h"

namespace elang {
namespace lir {

class CodeBuffer;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// CodeBufferUser
//
class CodeBufferUser {
 protected:
  explicit CodeBufferUser(CodeBuffer* code_buffer);
  ~CodeBufferUser();

  CodeBuffer* code_buffer() const { return code_buffer_; }

  void AssociateCallSite(base::StringPiece16 callee);
  void AssociateValue(Value value);
  void Emit16(int data);
  void Emit32(uint32_t data);
  void Emit64(uint64_t data);
  void Emit8(int data);

 private:
  CodeBuffer* const code_buffer_;

  DISALLOW_COPY_AND_ASSIGN(CodeBufferUser);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_CODE_BUFFER_USER_H_
