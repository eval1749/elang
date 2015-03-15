// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/emitters/code_buffer_user.h"

#include "elang/lir/emitters/code_buffer.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// CodeBufferUser
//
CodeBufferUser::CodeBufferUser(CodeBuffer* code_buffer)
    : code_buffer_(code_buffer) {
}

CodeBufferUser::~CodeBufferUser() {
}

void CodeBufferUser::AssociateCallSite(base::StringPiece16 callee) {
  code_buffer_->AssociateCallSite(callee);
}

void CodeBufferUser::AssociateValue(Value value) {
  code_buffer_->AssociateValue(value);
}

void CodeBufferUser::Emit16(int data) {
  code_buffer_->Emit16(data);
}

void CodeBufferUser::Emit32(uint32_t data) {
  code_buffer_->Emit32(data);
}

void CodeBufferUser::Emit64(uint64_t data) {
  code_buffer_->Emit64(data);
}

void CodeBufferUser::Emit8(int data) {
  code_buffer_->Emit8(data);
}

}  // namespace lir
}  // namespace elang
