// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_allocator.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/lir/analysis/conflict_map.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
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

  // Put parameters into free slot list for assigning them as spill slot in
  // 'copy' and 'pcopy' after 'entry' instruction.
  for (auto const parameter : editor->function()->parameters()) {
    if (!parameter.is_parameter())
      continue;
    auto const slot = new (zone()) Slot(zone());
    slot->proxy = parameter;
    free_slots_.insert(slot);
  }
}

StackAllocator::~StackAllocator() {
}

Value StackAllocator::AllocationFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = slot_map_.find(vreg);
  return it == slot_map_.end() ? Value() : it->second->proxy;
}

Value StackAllocator::Allocate(Value vreg) {
  DCHECK(vreg.is_virtual());
  DCHECK(!slot_map_.count(vreg));

  // Find reusable free slot for |vreg|.
  if (auto const slot = FreeSlotFor(vreg)) {
    free_slots_.erase(slot);
    live_slots_.insert(slot);
    slot_map_[vreg] = slot;
    slot->users.push_back(vreg);
    return slot->proxy;
  }

  // There are no reusable free slots, we use new slot for |vreg|.
  auto const slot = NewSlot(vreg);
  slot->users.push_back(vreg);
  slot_map_[vreg] = slot;
  return slot->proxy;
}

void StackAllocator::AllocateForPreserving(Value physical) {
  DCHECK(physical.is_physical());
  auto const natural = Target::NaturalRegisterOf(physical);
  if (assignments_->preserving_registers_.count(natural)) {
    // We've already have slot for preserving register.
    return;
  }

  for (auto const slot : free_slots_) {
    if (slot->proxy.size == natural.size) {
      free_slots_.erase(slot);
      assignments_->preserving_registers_[natural] = slot->proxy;
      live_slots_.insert(slot);
      return;
    }
  }

  // There are no reusable free slots, we use new slot for |natural|.
  auto const slot = NewSlot(natural);
  assignments_->preserving_registers_[natural] = slot->proxy;
  live_slots_.insert(slot);
}

// Allocate stack by offset and size specified by |proxy|. This function
// may be called after |Reset()|.
void StackAllocator::Assign(Value vreg, Value proxy) {
  DCHECK(vreg.is_virtual());
  DCHECK(proxy.is_memory_proxy());
  DCHECK(vreg.is_virtual());
  DCHECK(!slot_map_.count(vreg));
  Slot* slot = nullptr;
  for (auto const present : free_slots_) {
    if (present->proxy == proxy) {
      slot = present;
      break;
    }
  }
  DCHECK(slot) << proxy << " for " << vreg << " is already used.";
  DCHECK(!live_slots_.count(slot));
  free_slots_.erase(slot);
  live_slots_.insert(slot);
  slot_map_[vreg] = slot;
  slot->users.push_back(vreg);
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

StackAllocator::Slot* StackAllocator::FreeSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  for (auto const slot : free_slots_) {
    if (slot->proxy.size == vreg.size) {
      if (!IsConflict(slot, vreg))
        return slot;
    }
  }
  return nullptr;
}

bool StackAllocator::IsConflict(const Slot* slot, Value vreg) const {
  DCHECK(vreg.is_virtual());
  for (auto const user : slot->users) {
    if (conflict_map_.IsConflict(user, vreg))
      return true;
  }
  return false;
}

StackAllocator::Slot* StackAllocator::NewSlot(Value type) {
  auto const offset = Align(size_, Value::ByteSizeOf(type));
  size_ += Value::ByteSizeOf(type);
  assignments_->maximum_variables_size_ = Align(size_, alignment_);
  auto const slot = new (zone()) Slot(zone());
  live_slots_.insert(slot);
  slot->proxy = Value::SpillSlot(type, offset);
  return slot;
}

// Allocate stack by offset and size specified by |proxy|. This function
// may be called after |Reset()|.
void StackAllocator::Reallocate(Value vreg, Value proxy) {
  DCHECK(vreg.is_virtual());
  DCHECK(proxy.is_memory_proxy());
  DCHECK(vreg.is_virtual());
  auto const it = slot_map_.find(vreg);
  DCHECK(it != slot_map_.end());
  auto const slot = it->second;
  DCHECK(free_slots_.count(slot)) << proxy << " for " << vreg
                                  << " is already used.";
  DCHECK(!live_slots_.count(slot));
  free_slots_.erase(slot);
  live_slots_.insert(slot);
}

void StackAllocator::Reset() {
  while (!live_slots_.empty()) {
    auto const slot = *live_slots_.begin();
    live_slots_.erase(slot);
    free_slots_.insert(slot);
  }
}

void StackAllocator::TrackArgument(Value argument) {
  DCHECK(argument.is_argument());
  assignments_->arguments_.insert(argument);
  assignments_->maximum_arguments_size_ =
      std::max(assignments_->maximum_arguments_size_,
               (argument.data + 1) * Value::ByteSizeOf(Target::IntPtrType()));
}

void StackAllocator::TrackCall(Instruction* instr) {
  DCHECK(instr->is<CallInstruction>());

  // Track arguments
  if (auto const previous = instr->previous()) {
    if (previous->is<CopyInstruction>() || previous->is<PCopyInstruction>()) {
      auto position = 0;
      for (auto const input : previous->inputs()) {
        TrackArgument(Value::Argument(input, position));
        ++position;
      }
    }
  }

  ++assignments_->number_of_calls_;
}

}  // namespace lir
}  // namespace elang
