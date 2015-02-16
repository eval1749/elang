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
    RegisterAllocation* allocations_map)
    : allocations_(allocations_map) {
}

RegisterAllocationTracker::~RegisterAllocationTracker() {
}

const std::unordered_map<Value, Value>&
RegisterAllocationTracker::physical_map() const {
  return physical_map_;
}

Value RegisterAllocationTracker::AllocationOf(BasicBlock* block,
                                              Value value) const {
  return allocations_->AllocationOf(block, value);
}

Value RegisterAllocationTracker::AllocationOf(Value virtual_register) const {
  auto const physical = PhysicalFor(virtual_register);
  if (physical.is_physical())
    return physical;
  return StackSlotFor(virtual_register);
}

void RegisterAllocationTracker::EndBlock(BasicBlock* block) {
  DCHECK(block);
  for (auto pair : physical_map_) {
    auto const physical = pair.second;
    if (!physical.is_physical())
      continue;
    allocations_->SetPhysical(block, pair.first, physical);
  }
}

void RegisterAllocationTracker::FreePhysical(Value physical) {
  DCHECK(physical.is_physical());
  for (auto it = physical_map_.begin(); it != physical_map_.end(); ++it) {
    if (it->second == physical) {
      physical_map_.erase(it);
      return;
    }
  }
  NOTREACHED() << "block map doesn't have " << physical;
}

void RegisterAllocationTracker::FreeVirtual(Value vreg) {
  DCHECK(vreg.is_virtual());
  DCHECK(PhysicalFor(vreg).is_physical() || StackSlotFor(vreg).is_stack_slot());
  auto const physical_it = physical_map_.find(vreg);
  if (physical_it != physical_map_.end())
    physical_map_.erase(physical_it);

  auto const stack_slot_it = allocations_->stack_slot_map_.find(vreg);
  if (stack_slot_it != allocations_->stack_slot_map_.end())
    allocations_->stack_slot_map_.erase(stack_slot_it);
}

void RegisterAllocationTracker::InsertBefore(Instruction* new_instr,
                                             Instruction* ref_instr) {
  allocations_->InsertBefore(new_instr, ref_instr);
}

Value RegisterAllocationTracker::PhysicalFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = physical_map_.find(vreg);
  return it == physical_map_.end() ? Value() : it->second;
}

Value RegisterAllocationTracker::StackSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = allocations_->stack_slot_map_.find(vreg);
  return it == allocations_->stack_slot_map_.end() ? Value() : it->second;
}

void RegisterAllocationTracker::StartBlock(BasicBlock* block) {
  DCHECK(block);
  physical_map_.clear();
}

void RegisterAllocationTracker::SetAllocation(Instruction* instr,
                                              Value vreg,
                                              Value allocation) {
  DCHECK(vreg.is_virtual());
  DCHECK_EQ(vreg.type, allocation.type);
  DCHECK_EQ(vreg.size, allocation.size);
  allocations_->SetAllocation(instr, vreg, allocation);
  if (allocation.is_physical()) {
    DCHECK_EQ(PhysicalFor(vreg), allocation);
    return;
  }
  if (allocation.is_stack_slot()) {
    DCHECK_EQ(StackSlotFor(vreg), allocation);
    return;
  }
  NOTREACHED() << "Unexpected allocation: " << allocation;
}

void RegisterAllocationTracker::SetPhysical(BasicBlock* block,
                                            Value vreg,
                                            Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  DCHECK_EQ(vreg.type, physical.type);
  DCHECK_EQ(vreg.size, physical.size);
  allocations_->SetPhysical(block, vreg, physical);
}

void RegisterAllocationTracker::TrackPhysical(Value vreg, Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  DCHECK_EQ(vreg.type, physical.type);
  DCHECK_EQ(vreg.size, physical.size);
  DCHECK(!physical_map_.count(vreg));
  DCHECK(VirtualFor(physical).is_void());
  physical_map_[vreg] = physical;
}

void RegisterAllocationTracker::TrackStackSlot(Value vreg, Value stack_slot) {
  DCHECK(vreg.is_virtual());
  DCHECK(stack_slot.is_stack_slot());
  DCHECK(!allocations_->stack_slot_map_.count(stack_slot));
  allocations_->stack_slot_map_[vreg] = stack_slot;
}

bool RegisterAllocationTracker::TryAllocate(Instruction* instr,
                                            Value vreg,
                                            Value physical) {
  DCHECK_EQ(vreg.type, physical.type);
  DCHECK_EQ(vreg.size, physical.size);
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
  for (auto const pair : physical_map_) {
    if (pair.second == physical)
      return pair.first;
  }
  return Value();
}

}  // namespace lir
}  // namespace elang
