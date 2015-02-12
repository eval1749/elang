// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_allocation.h"

#include "base/logging.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LocalAllocation
//
LocalAllocation::LocalAllocation(Zone* zone)
    : physical_map_(zone), stack_slot_map_(zone) {
}

Value LocalAllocation::PhysicalFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = physical_map_.find(vreg);
  return it == physical_map_.end() ? Value() : it->second;
}

Value LocalAllocation::StackSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = stack_slot_map_.find(vreg);
  return it == stack_slot_map_.end() ? Value() : it->second;
}

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocation::Actions
//
RegisterAllocation::Actions::Actions(Zone* zone,
                                     const std::vector<Instruction*> actions)
    : actions(zone, actions) {
}

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocation
//
RegisterAllocation::RegisterAllocation() {
}

RegisterAllocation::~RegisterAllocation() {
}

const LocalAllocation& RegisterAllocation::AllocationOf(
    BasicBlock* block) const {
  auto const it = block_map_.find(block);
  DCHECK(it != block_map_.end());
  return *it->second;
}

Value RegisterAllocation::AllocationOf(Instruction* instr, Value vreg) const {
  auto const it = map_.find(std::make_pair(instr, vreg));
  DCHECK(it != map_.end());
  return it->second;
}

const ZoneVector<Instruction*>& RegisterAllocation::BeforeActionOf(
    Instruction* instr) const {
  auto const it = before_action_map_.find(instr);
  DCHECK(it != before_action_map_.end());
  return it->second->actions;
}

void RegisterAllocation::SetAllocation(Instruction* instr,
                                       Value vreg,
                                       Value allocated) {
  DCHECK(vreg.is_virtual());
  DCHECK(allocated.is_physical() || allocated.is_stack_slot());
  map_[std::make_pair(instr, vreg)] = allocated;
}

void RegisterAllocation::SetBeforeAction(
    Instruction* instr,
    const std::vector<Instruction*>& actions) {
  DCHECK(!before_action_map_.count(instr));
  before_action_map_[instr] = new (zone()) Actions(zone(), actions);
}

}  // namespace lir
}  // namespace elang

namespace std {
size_t hash<elang::lir::ValueLocation>::operator()(
    const elang::lir::ValueLocation& location) const {
  return std::hash<elang::lir::Instruction*>()(location.first) ^
         std::hash<elang::lir::Value>()(location.second);
}
}  // namespace std
