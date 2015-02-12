// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_allocator.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
int RoundUp(int value, int alignment) {
  return (value + alignment - 1) / alignment * alignment;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// StackAllocator
//
StackAllocator::StackAllocator(int alignment) : alignment_(alignment) {
  DCHECK(alignment_ == 4 || alignment_ == 8 || alignment_ == 16);
  // Reserve spaces to reduce number of dynamic expansion.
  uses_.reserve(alignment_ * 32);
}

StackAllocator::~StackAllocator() {
}

int StackAllocator::Allocate(int size) {
  auto const end = uses_.end();
  auto runner = uses_.begin();
  while (runner != end) {
    auto const candidate = std::find(runner, end, false);
    if (candidate == end)
      break;
    auto const offset = static_cast<int>(candidate - uses_.begin());
    if (offset % size) {
      ++runner;
      continue;
    }
    auto const next_use = std::find(candidate + 1, end, true);
    if (next_use - candidate >= size) {
      std::fill(candidate, candidate + size, true);
      return offset;
    }
    runner = next_use;
  }
  // We expand allocation map since there are no free slots for |size|.
  auto const offset = static_cast<int>(uses_.size());
  uses_.resize(offset + RoundUp(size, alignment_));
  std::fill(uses_.begin() + offset, uses_.begin() + offset + size, true);
  return offset;
}

Value StackAllocator::Allocate(Value type) {
  return Value::Stack(type, Allocate(Value::ByteSize(type.size)));
}

// Allocate stack by offset and size specified by |stack_slot|. This function
// may be called after |Reset()|.
void StackAllocator::AllocateAt(Value stack_slot) {
  DCHECK(stack_slot.is_stack());
  auto const offset = stack_slot.data;
  auto const size = Value::ByteSize(stack_slot.size);
  if (offset + size > static_cast<int>(uses_.size()))
    uses_.resize(offset + size);
#ifndef _NDEBUG
  for (auto index = offset; index < offset + size; ++index)
    DCHECK(!uses_[index]);
#endif
  std::fill(uses_.begin() + offset, uses_.begin() + offset + size, true);
}

void StackAllocator::Free(Value location) {
  DCHECK_EQ(Value::Kind::Stack, location.kind);
  std::fill(uses_.begin() + location.data,
            uses_.begin() + location.data + Value::ByteSize(location.size),
            false);
}

int StackAllocator::RequiredSize() const {
  return static_cast<int>(uses_.size());
}

void StackAllocator::Reset() {
  uses_.resize(0);
}

}  // namespace lir
}  // namespace elang
