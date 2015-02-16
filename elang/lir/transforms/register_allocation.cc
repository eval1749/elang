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
// RegisterAllocation::Actions
//
RegisterAllocation::Actions::Actions(Zone* zone) : actions(zone) {
}

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocation
//
RegisterAllocation::RegisterAllocation()
    : block_value_map_(zone()),
      before_action_map_(zone()),
      empty_actions_(zone()),
      instruction_value_map_(zone()),
      stack_slot_map_(zone()) {
}

RegisterAllocation::~RegisterAllocation() {
}

Value RegisterAllocation::AllocationOf(BasicBlock* block, Value value) const {
  if (!value.is_virtual())
    return value;
  auto const it = block_value_map_.find(std::make_pair(block, value));
  DCHECK(it != block_value_map_.end());
  return it->second;
}

Value RegisterAllocation::AllocationOf(Instruction* instr, Value value) const {
  if (!value.is_virtual())
    return value;
  auto const it = instruction_value_map_.find(std::make_pair(instr, value));
  DCHECK(it != instruction_value_map_.end());
  return it->second;
}

const ZoneVector<Instruction*>& RegisterAllocation::BeforeActionOf(
    Instruction* instr) const {
  auto const it = before_action_map_.find(instr);
  return it == before_action_map_.end() ? empty_actions_ : it->second->actions;
}

void RegisterAllocation::InsertBefore(Instruction* new_instr,
                                      Instruction* ref_instr) {
  auto const it = before_action_map_.find(ref_instr);
  if (it == before_action_map_.end()) {
    auto const actions = new (zone()) Actions(zone());
    actions->actions.push_back(new_instr);
    before_action_map_[ref_instr] = actions;
    return;
  }
  it->second->actions.push_back(new_instr);
}

void RegisterAllocation::SetAllocation(Instruction* instr,
                                       Value vreg,
                                       Value allocation) {
  DCHECK(vreg.is_virtual());
  DCHECK(allocation.is_physical() || allocation.is_stack_slot());
  instruction_value_map_[std::make_pair(instr, vreg)] = allocation;
}

void RegisterAllocation::SetPhysical(BasicBlock* block,
                                     Value vreg,
                                     Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  block_value_map_[std::make_pair(block, vreg)] = physical;
}

void RegisterAllocation::SetStackSlot(Value vreg, Value stack_slot) {
  DCHECK(vreg.is_virtual());
  DCHECK(stack_slot.is_stack_slot());
  DCHECK(!stack_slot_map_.count(vreg));
  stack_slot_map_[vreg] = stack_slot;
}

}  // namespace lir
}  // namespace elang

namespace std {
size_t hash<elang::lir::BasicBlockValue>::operator()(
    const elang::lir::BasicBlockValue& location) const {
  return std::hash<elang::lir::BasicBlock*>()(location.first) ^
         std::hash<elang::lir::Value>()(location.second);
}

size_t hash<elang::lir::InstructionValue>::operator()(
    const elang::lir::InstructionValue& location) const {
  return std::hash<elang::lir::Instruction*>()(location.first) ^
         std::hash<elang::lir::Value>()(location.second);
}
}  // namespace std
