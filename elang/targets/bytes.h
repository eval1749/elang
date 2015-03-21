// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_TARGETS_BYTES_H_
#define ELANG_TARGETS_BYTES_H_

#include "base/basictypes.h"

namespace elang {
namespace targets {

//////////////////////////////////////////////////////////////////////
//
// Bytes
//
class Bytes final {
 public:
  Bytes(uint8_t* bytes, size_t size);
  ~Bytes();

  const uint8_t* bytes() const { return bytes_; }
  size_t size() const { return size_; }

  void SetBytes(size_t offset, const uint8_t* data, size_t size);
  void SetInt16(size_t offset, int data);
  void SetInt32(size_t offset, int32_t data);
  void SetInt64(size_t offset, int64_t data);
  void SetInt8(size_t offset, int data);
  void SetRelativeAddress32(size_t offset, const uint8_t* target_address);
  void SetUInt16(size_t offset, int data);
  void SetUInt32(size_t offset, uint32_t data);
  void SetUInt64(size_t offset, uint64_t data);
  void SetUInt8(size_t offset, int data);

 private:
  uint8_t* const bytes_;
  size_t const size_;

  DISALLOW_COPY_AND_ASSIGN(Bytes);
};

}  // namespace targets
}  // namespace elang

#endif  // ELANG_TARGETS_BYTES_H_
