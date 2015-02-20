// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_allocator.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/lir/transforms/stack_assignments.h"
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
StackAllocator::StackAllocator(StackAssignments* assignments, int alignment)
    : alignment_(alignment), assignments_(assignments) {
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
  auto const offset = current_size();
  uses_.resize(offset + RoundUp(size, alignment_));
  std::fill(uses_.begin() + offset, uses_.begin() + offset + size, true);
  assignments_->maximum_size_ = current_size();
  return offset;
}

Value StackAllocator::Allocate(Value type) {
  return Value::SpillSlot(type, Allocate(Value::ByteSizeOf(type.size)));
}

// Allocate stack by offset and size specified by |spill_slot|. This function
// may be called after |Reset()|.
void StackAllocator::AllocateAt(Value spill_slot) {
  DCHECK(spill_slot.is_spill_slot());
  auto const offset = spill_slot.data;
  auto const size = Value::ByteSizeOf(spill_slot.size);
  if (offset + size > current_size())
    uses_.resize(offset + size);
#ifndef _NDEBUG
  for (auto index = offset; index < offset + size; ++index)
    DCHECK(!uses_[index]);
#endif
  std::fill(uses_.begin() + offset, uses_.begin() + offset + size, true);
}

void StackAllocator::Free(Value location) {
  DCHECK(location.is_spill_slot()) << location;
  std::fill(uses_.begin() + location.data,
            uses_.begin() + location.data + Value::ByteSizeOf(location.size),
            false);
}

int StackAllocator::RequiredSize() const {
  return std::max(assignments_->maximum_size_, current_size());
}

void StackAllocator::Reset() {
  assignments_->maximum_size_ =
      std::max(assignments_->maximum_size_, current_size());
  uses_.resize(0);
}

void StackAllocator::TrackNumberOfArguments(int argc) {
  DCHECK_GE(argc, 0);
  assignments_->maximum_argc_ = std::max(assignments_->maximum_argc_, argc);
  ++assignments_->number_of_calls_;
}

}  // namespace lir
}  // namespace elang
