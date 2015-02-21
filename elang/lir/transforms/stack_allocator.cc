// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_allocator.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/lir/analysis/conflict_map.h"
#include "elang/lir/editor.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
int Align(int value, int alignment) {
  return (value + alignment - 1) / alignment * alignment;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// StackAllocator
//
StackAllocator::StackAllocator(const Editor* editor,
                               StackAssignments* assignments)
    : alignment_(Value::ByteSizeOf(Target::IntPtrType())),
      assignments_(assignments),
      conflict_map_(editor->AnalyzeConflicts()),
      size_(0) {
  DCHECK(alignment_ == 4 || alignment_ == 8 || alignment_ == 16);
}

StackAllocator::~StackAllocator() {
}

Value StackAllocator::AllocationFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = slot_map_.find(vreg);
  return it == slot_map_.end() ? Value() : it->second->spill_slot;
}

Value StackAllocator::Allocate(Value vreg) {
  DCHECK(vreg.is_virtual());
  DCHECK(!slot_map_.count(vreg));

  // Find reusable free slot for |vreg|.
  for (auto const slot : free_slots_) {
    if (slot->spill_slot.size == vreg.size) {
      if (!IsConflict(slot, vreg)) {
        free_slots_.erase(slot);
        live_slots_.insert(slot);
        slot_map_[vreg] = slot;
        slot->users.push_back(vreg);
        return slot->spill_slot;
      }
    }
  }

  // There are no reusable free slots, we allocate new slot for |vreg|.
  auto const offset = Align(size_, Value::ByteSizeOf(vreg));
  size_ += Value::ByteSizeOf(vreg);
  assignments_->maximum_size_ = std::max(assignments_->maximum_size_,
                                         Align(size_, alignment_));
  auto const slot = new (zone()) Slot(zone());
  live_slots_.insert(slot);
  slot->spill_slot = Value::SpillSlot(vreg, offset);
  slot->users.push_back(vreg);
  slot_map_[vreg] = slot;
  return slot->spill_slot;
}

// Allocate stack by offset and size specified by |spill_slot|. This function
// may be called after |Reset()|.
void StackAllocator::Assign(Value vreg, Value spill_slot) {
  DCHECK(vreg.is_virtual());
  DCHECK(spill_slot.is_spill_slot());
  DCHECK(vreg.is_virtual());
  auto const it = slot_map_.find(vreg);
  DCHECK(it != slot_map_.end());
  auto const slot = it->second;
  DCHECK(free_slots_.count(slot)) << spill_slot << " for " << vreg
                                  << " is already used.";
  DCHECK(!live_slots_.count(slot));
  free_slots_.erase(slot);
  live_slots_.insert(slot);
}

void StackAllocator::Free(Value vreg) {
  DCHECK(vreg.is_virtual());
  auto const it = slot_map_.find(vreg);
  DCHECK(it != slot_map_.end());
  auto const slot = it->second;
  DCHECK(live_slots_.count(slot));
  live_slots_.erase(slot);
  free_slots_.insert(slot);
}

bool StackAllocator::IsConflict(const Slot* slot, Value vreg) const {
  DCHECK(vreg.is_virtual());
  for (auto const user : slot->users) {
    if (conflict_map_.IsConflict(user, vreg))
      return true;
  }
  return false;
}

void StackAllocator::Reset() {
  while (!live_slots_.empty()) {
    auto const slot = *live_slots_.begin();
    live_slots_.erase(slot);
    free_slots_.insert(slot);
  }
}

void StackAllocator::TrackNumberOfArguments(int argc) {
  DCHECK_GE(argc, 0);
  assignments_->maximum_argc_ = std::max(assignments_->maximum_argc_, argc);
  ++assignments_->number_of_calls_;
}

}  // namespace lir
}  // namespace elang
