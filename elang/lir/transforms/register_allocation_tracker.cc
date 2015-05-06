// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/register_allocation_tracker.h"

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
bool EqualsIgnoringSize(Value physical1, Value physical2) {
  DCHECK(physical1.is_physical());
  DCHECK(physical2.is_physical());
  DCHECK_EQ(physical1.type, physical2.type);
  return physical1.data == physical2.data;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocationTracker
//
RegisterAllocationTracker::RegisterAllocationTracker(
    RegisterAssignments* assignments)
    : assignments_(assignments) {
}

RegisterAllocationTracker::~RegisterAllocationTracker() {
}

const std::unordered_map<Value, Value>&
RegisterAllocationTracker::physical_map() const {
  return physical_map_;
}

Value RegisterAllocationTracker::AllocationOf(BasicBlock* block,
                                              Value value) const {
  return assignments_.AllocationOf(block, value);
}

Value RegisterAllocationTracker::AllocationOf(Instruction* instr,
                                              Value value) const {
  return assignments_.AllocationOf(instr, value);
}

Value RegisterAllocationTracker::AllocationOf(Value virtual_register) const {
  auto const physical = PhysicalFor(virtual_register);
  if (physical.is_physical())
    return physical;
  return SpillSlotFor(virtual_register);
}

void RegisterAllocationTracker::EndBlock(BasicBlock* block) {
  DCHECK(block);
  for (auto pair : physical_map_) {
    auto const physical = pair.second;
    if (!physical.is_physical())
      continue;
    assignments_.SetPhysical(block, pair.first, physical);
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
  NOTREACHED() << "No entry for " << physical << std::endl
               << physical_map_;
}

void RegisterAllocationTracker::FreeVirtual(Value vreg) {
  DCHECK(vreg.is_virtual());
  DCHECK(PhysicalFor(vreg).is_physical() ||
         SpillSlotFor(vreg).is_memory_proxy());
  auto const physical_it = physical_map_.find(vreg);
  if (physical_it != physical_map_.end())
    physical_map_.erase(physical_it);
}

void RegisterAllocationTracker::InsertBefore(Instruction* new_instr,
                                             Instruction* ref_instr) {
  assignments_.InsertBefore(new_instr, ref_instr);
}

Value RegisterAllocationTracker::PhysicalFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = physical_map_.find(vreg);
  return it == physical_map_.end() ? Value::Void() : it->second;
}

void RegisterAllocationTracker::SetAllocation(Instruction* instr,
                                              Value vreg,
                                              Value allocation) {
  DCHECK(vreg.is_virtual());
  DCHECK_EQ(Value::TypeOf(vreg), Value::TypeOf(allocation)) << vreg << " "
                                                            << allocation;
  assignments_.SetAllocation(instr, vreg, allocation);
  if (allocation.is_physical()) {
    DCHECK_EQ(PhysicalFor(vreg), allocation);
    return;
  }
  if (allocation.is_memory_proxy()) {
    DCHECK_EQ(SpillSlotFor(vreg), allocation);
    return;
  }
  NOTREACHED() << "Unexpected allocation: " << allocation;
}

void RegisterAllocationTracker::SetPhysical(BasicBlock* block,
                                            Value vreg,
                                            Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  DCHECK_EQ(Value::TypeOf(vreg), Value::TypeOf(physical)) << vreg << " "
                                                          << physical;
  assignments_.SetPhysical(block, vreg, physical);
}

void RegisterAllocationTracker::SetSpillSlot(Value vreg, Value spill_slot) {
  DCHECK(vreg.is_virtual());
  DCHECK(spill_slot.is_memory_proxy());
  assignments_.SetSpillSlot(vreg, spill_slot);
}

Value RegisterAllocationTracker::SpillSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  return assignments_.SpillSlotFor(vreg);
}

void RegisterAllocationTracker::StartBlock(BasicBlock* block) {
  DCHECK(block);
  physical_map_.clear();
}

void RegisterAllocationTracker::TrackPhysical(Value vreg, Value physical) {
  DCHECK(vreg.is_virtual()) << vreg;
  DCHECK(physical.is_physical()) << physical;
  DCHECK_EQ(Value::TypeOf(vreg), Value::TypeOf(physical)) << vreg << " "
                                                          << physical;
  DCHECK(!physical_map_.count(vreg)) << vreg << " " << physical_map_[vreg];
  DCHECK(VirtualFor(physical).is_void())
      << "Can't allocate " << vreg << " to " << physical
      << ", it is already allocated to " << VirtualFor(physical);
  physical_map_[vreg] = physical;
}

Value RegisterAllocationTracker::VirtualFor(Value physical) const {
  DCHECK(physical.is_physical());
  for (auto const pair : physical_map_) {
    if (EqualsIgnoringSize(pair.second, physical))
      return pair.first;
  }
  return Value::Void();
}

}  // namespace lir
}  // namespace elang
