// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "elang/targets/bytes.h"

#include "base/logging.h"
#include "elang/targets/target_features.h"

namespace elang {
namespace targets {

Bytes::Bytes(uint8_t* bytes, size_t size) : bytes_(bytes), size_(size) {
}

Bytes::~Bytes() {
}

void Bytes::SetBytes(size_t offset, const uint8_t* bytes, size_t size) {
  DCHECK_LE(offset + size, size_);
  ::memcpy(bytes_ + offset, bytes, size);
}

void Bytes::SetInt16(size_t offset, int data) {
  DCHECK_LE(offset + 2, size_);
  DCHECK_GE(data, std::numeric_limits<int16_t>::min());
  DCHECK_LE(data, std::numeric_limits<int16_t>::max());
#if ELANG_TARGET_LITTLE_ENDIAN
  bytes_[offset] = static_cast<uint8_t>(data);
  bytes_[offset + 1] = static_cast<uint8_t>(data >> 8);
#else
  bytes_[offset] = static_cast<uint8_t>(data >> 8);
  bytes_[offset + 1] = static_cast<uint8_t>(data);
#endif
}

void Bytes::SetInt32(size_t offset, int32_t data) {
  SetUInt32(offset, static_cast<uint32_t>(data));
}

void Bytes::SetInt64(size_t offset, int64_t data) {
  SetUInt64(offset, static_cast<uint64_t>(data));
}

void Bytes::SetInt8(size_t offset, int data) {
  DCHECK_LE(offset + 1, size_);
  DCHECK_GE(data, std::numeric_limits<int8_t>::min());
  DCHECK_LE(data, std::numeric_limits<int8_t>::max());
  bytes_[offset] = static_cast<uint8_t>(data);
}

void Bytes::SetRelativeAddress32(size_t offset, const uint8_t* target_address) {
  auto const displacement = target_address - (bytes_ + offset + 4);
  DCHECK_LE(displacement, static_cast<decltype(displacement)>(
                              std::numeric_limits<uint32_t>::max()));
  SetInt32(offset, static_cast<int32_t>(displacement));
}

void Bytes::SetUInt16(size_t offset, int data) {
  DCHECK_LE(offset + 2, size_);
  DCHECK_LE(data, std::numeric_limits<uint16_t>::max());
#if ELANG_TARGET_LITTLE_ENDIAN
  bytes_[offset] = static_cast<uint8_t>(data);
  bytes_[offset + 1] = static_cast<uint8_t>(data >> 8);
#else
  bytes_[offset] = static_cast<uint8_t>(data >> 8);
  bytes_[offset + 1] = static_cast<uint8_t>(data);
#endif
}

void Bytes::SetUInt32(size_t offset, uint32_t data) {
  DCHECK_LE(offset + 4, size_);
#if ELANG_TARGET_LITTLE_ENDIAN
  bytes_[offset] = static_cast<uint8_t>(data);
  bytes_[offset + 1] = static_cast<uint8_t>(data >> 8);
  bytes_[offset + 2] = static_cast<uint8_t>(data >> 16);
  bytes_[offset + 3] = static_cast<uint8_t>(data >> 24);
#else
  bytes_[offset] = static_cast<uint8_t>(data >> 24);
  bytes_[offset + 1] = static_cast<uint8_t>(data >> 16);
  bytes_[offset + 2] = static_cast<uint8_t>(data >> 8);
  bytes_[offset + 3] = static_cast<uint8_t>(data);
#endif
}

void Bytes::SetUInt64(size_t offset, uint64_t data) {
  DCHECK_LE(offset + 8, size_);
#if ELANG_TARGET_LITTLE_ENDIAN
  SetUInt32(offset, static_cast<uint32_t>(data >> 32));
  SetUInt32(offset + 4, static_cast<uint32_t>(data));
#else
  SetUInt32(offset, static_cast<uint32_t>(data));
  SetUInt32(offset + 4, static_cast<uint32_t>(data >> 32));
#endif
}

void Bytes::SetUInt8(size_t offset, int data) {
  DCHECK_LE(offset + 1, size_);
  DCHECK_LE(data, std::numeric_limits<uint8_t>::max());
  bytes_[offset] = static_cast<uint8_t>(data);
}

}  // namespace targets
}  // namespace elang
