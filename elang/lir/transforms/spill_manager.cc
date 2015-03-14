// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/spill_manager.h"

#include "base/logging.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/transforms/register_allocation_tracker.h"
#include "elang/lir/transforms/register_usage_tracker.h"
#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
struct SpillVictim {
  Value vreg;
  int next_use;

  SpillVictim() : next_use(0) {}
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// SpillManager
//
SpillManager::SpillManager(Factory* factory,
                           RegisterAllocationTracker* allocation_tracker,
                           StackAllocator* stack_allocator,
                           RegisterUsageTracker* usage_tracker)
    : allocation_tracker_(allocation_tracker),
      factory_(factory),
      stack_allocator_(stack_allocator),
      usage_tracker_(usage_tracker) {
}

SpillManager::~SpillManager() {
}

// Returns farthest used virtual register which types is |type| or farthest
// used virtual register.
Value SpillManager::ChooseRegisterToSpill(Value type,
                                          Instruction* instr) const {
  SpillVictim victim;
  SpillVictim spilled_victim;
  for (auto const it : allocation_tracker_->physical_map()) {
    auto const physical = it.second;
    if (physical.type != type.type)
      continue;
    auto const candidate = it.first;
    auto const next_use = usage_tracker_->NextUseAfter(candidate, instr);
    if (!next_use || victim.next_use < next_use->index()) {
      victim.next_use = next_use->index();
      victim.vreg = candidate;
    }
    if (SpillSlotFor(candidate).is_memory_proxy() &&
        (!next_use || spilled_victim.next_use < next_use->index())) {
      spilled_victim.next_use = next_use->index();
      spilled_victim.vreg = candidate;
    }
  }
  if (spilled_victim.vreg.is_virtual())
    return spilled_victim.vreg;

#ifndef _NDEBUG
  if (!victim.vreg.is_virtual()) {
    DVLOG(0) << "physical map for " << type << " at " << *instr;
    for (auto const it : allocation_tracker_->physical_map())
      DVLOG(0) << it.first << " -> " << it.second;
    NOTREACHED();
  }
#endif

  return victim.vreg;
}

Value SpillManager::EnsureSpillSlot(Value vreg) {
  DCHECK(vreg.is_virtual());
  auto const present = allocation_tracker_->SpillSlotFor(vreg);
  if (present.is_memory_proxy())
    return present;
  auto const spill_slot = stack_allocator_->Allocate(vreg);
  allocation_tracker_->SetSpillSlot(vreg, spill_slot);
  return spill_slot;
}

Instruction* SpillManager::NewReload(Value physical, Value vreg) {
  DCHECK(physical.is_physical());
  DCHECK(vreg.is_virtual());
  auto const spill_slot = allocation_tracker_->SpillSlotFor(vreg);
  DCHECK(spill_slot.is_memory_proxy());
  return factory_->NewCopyInstruction(physical, spill_slot);
}

Instruction* SpillManager::NewSpill(Value vreg, Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  auto const spill_slot = EnsureSpillSlot(vreg);
  return factory_->NewCopyInstruction(spill_slot, physical);
}

Value SpillManager::SpillSlotFor(Value vreg) const {
  return allocation_tracker_->SpillSlotFor(vreg);
}

}  // namespace lir
}  // namespace elang
