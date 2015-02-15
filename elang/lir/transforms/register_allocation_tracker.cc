// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/register_allocation_tracker.h"

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/transforms/register_allocation.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocationTracker
//
RegisterAllocationTracker::RegisterAllocationTracker(
    RegisterAllocation* allocation_map)
    : register_allocation_(allocation_map), location_allocation_(nullptr) {
}

RegisterAllocationTracker::~RegisterAllocationTracker() {
}

const ZoneUnorderedMap<Value, Value>& RegisterAllocationTracker::physical_map()
    const {
  return location_allocation_->physical_map_;
}

const LocalAllocation& RegisterAllocationTracker::AllocationsOf(
    BasicBlock* block) const {
  return register_allocation_->AllocationsOf(block);
}

Value RegisterAllocationTracker::AllocationOf(Value virtual_register) const {
  auto const physical = PhysicalFor(virtual_register);
  if (physical.is_physical())
    return physical;
  return StackSlotFor(virtual_register);
}

void RegisterAllocationTracker::EndBlock(BasicBlock* block) {
  DCHECK(block);
  // TODO(eval1749) record elapsed time for |block|.
  location_allocation_ = nullptr;
}

void RegisterAllocationTracker::FreePhysical(Value physical) {
  DCHECK(physical.is_physical());
  for (auto it = location_allocation_->physical_map_.begin();
       it != location_allocation_->physical_map_.end(); ++it) {
    if (it->second == physical) {
      location_allocation_->physical_map_.erase(it);
      return;
    }
  }
  NOTREACHED() << "block map doesn't have " << physical;
}

void RegisterAllocationTracker::FreeVirtual(Value vreg) {
  DCHECK(vreg.is_virtual());
  DCHECK(location_allocation_->physical_map_.count(vreg) ||
         location_allocation_->stack_slot_map_.count(vreg));
  auto const physical_it = location_allocation_->physical_map_.find(vreg);
  if (physical_it != location_allocation_->physical_map_.end())
    location_allocation_->physical_map_.erase(physical_it);

  auto const stack_slot_it = location_allocation_->stack_slot_map_.find(vreg);
  if (stack_slot_it != location_allocation_->stack_slot_map_.end())
    location_allocation_->stack_slot_map_.erase(stack_slot_it);
}

void RegisterAllocationTracker::InsertBefore(Instruction* new_instr,
                                             Instruction* ref_instr) {
  register_allocation_->InsertBefore(new_instr, ref_instr);
}

Value RegisterAllocationTracker::PhysicalFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = location_allocation_->physical_map_.find(vreg);
  return it == location_allocation_->physical_map_.end() ? Value() : it->second;
}

Value RegisterAllocationTracker::StackSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = location_allocation_->stack_slot_map_.find(vreg);
  return it == location_allocation_->stack_slot_map_.end() ? Value()
                                                           : it->second;
}

void RegisterAllocationTracker::StartBlock(BasicBlock* block) {
  DCHECK(!register_allocation_->block_map_.count(block));
  DCHECK(!location_allocation_);
  auto const zone = register_allocation_->zone();
  location_allocation_ = new (zone) LocalAllocation(zone);
  register_allocation_->block_map_[block] = location_allocation_;
}

void RegisterAllocationTracker::SetAllocation(Instruction* instr,
                                              Value vreg,
                                              Value allocated) {
  DCHECK(vreg.is_virtual());
  register_allocation_->SetAllocation(instr, vreg, allocated);
  if (allocated.is_physical()) {
    DCHECK_EQ(PhysicalFor(vreg), allocated);
    return;
  }
  if (allocated.is_stack_slot()) {
    DCHECK_EQ(StackSlotFor(vreg), allocated);
    return;
  }
  NOTREACHED() << "Unexpected allocation: " << allocated;
}

void RegisterAllocationTracker::TrackPhysical(Value vreg, Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  DCHECK(!location_allocation_->physical_map_.count(vreg));
  DCHECK(VirtualFor(physical).is_void());
  location_allocation_->physical_map_[vreg] = physical;
}

void RegisterAllocationTracker::TrackStackSlot(Value vreg, Value stack_slot) {
  DCHECK(vreg.is_virtual());
  DCHECK(stack_slot.is_stack_slot());
  DCHECK(!location_allocation_->stack_slot_map_.count(stack_slot));
  location_allocation_->stack_slot_map_[vreg] = stack_slot;
}

bool RegisterAllocationTracker::TryAllocate(Instruction* instr,
                                            Value vreg,
                                            Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  auto const present = VirtualFor(physical);
  if (present.is_virtual()) {
    DCHECK_NE(present, vreg);
    return false;
  }
  TrackPhysical(vreg, physical);
  SetAllocation(instr, vreg, physical);
  return true;
}

Value RegisterAllocationTracker::VirtualFor(Value physical) const {
  DCHECK(physical.is_physical());
  for (auto const pair : location_allocation_->physical_map_) {
    if (pair.second == physical)
      return pair.first;
  }
  return Value();
}

}  // namespace lir
}  // namespace elang
