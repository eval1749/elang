// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_assignments.h"

#include "base/logging.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// RegisterAssignments::Actions
//
RegisterAssignments::Actions::Actions(Zone* zone) : actions(zone) {
}

//////////////////////////////////////////////////////////////////////
//
// RegisterAssignments::Editor
//
RegisterAssignments::Editor::Editor(RegisterAssignments* assignments)
    : assignments_(assignments) {
}

RegisterAssignments::Editor::~Editor() {
}

const ZoneUnorderedMap<Value, Value>&
RegisterAssignments::Editor::stack_slot_map() const {
  return assignments_->stack_slot_map_;
}

Zone* RegisterAssignments::Editor::zone() const {
  return assignments_->zone();
}

Value RegisterAssignments::Editor::AllocationOf(BasicBlock* block,
                                                Value value) const {
  return assignments_->AllocationOf(block, value);
}

Value RegisterAssignments::Editor::AllocationOf(Instruction* instr,
                                                Value value) const {
  return assignments_->AllocationOf(instr, value);
}

void RegisterAssignments::Editor::InsertBefore(Instruction* new_instr,
                                               Instruction* ref_instr) {
  auto const it = assignments_->before_action_map_.find(ref_instr);
  if (it == assignments_->before_action_map_.end()) {
    auto const actions = new (zone()) Actions(zone());
    actions->actions.push_back(new_instr);
    assignments_->before_action_map_[ref_instr] = actions;
    return;
  }
  it->second->actions.push_back(new_instr);
}

void RegisterAssignments::Editor::SetAllocation(Instruction* instr,
                                                Value vreg,
                                                Value allocation) {
  DCHECK(vreg.is_virtual());
  DCHECK(allocation.is_physical() || allocation.is_stack_slot());
  assignments_->instruction_value_map_[std::make_pair(instr, vreg)] =
      allocation;
}

void RegisterAssignments::Editor::SetPhysical(BasicBlock* block,
                                              Value vreg,
                                              Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  assignments_->block_value_map_[std::make_pair(block, vreg)] = physical;
}

void RegisterAssignments::Editor::SetStackSlot(Value vreg, Value stack_slot) {
  DCHECK(vreg.is_virtual());
  DCHECK(stack_slot.is_stack_slot());
  DCHECK(!assignments_->stack_slot_map_.count(vreg));
  assignments_->stack_slot_map_[vreg] = stack_slot;
}

Value RegisterAssignments::Editor::StackSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  return assignments_->StackSlotFor(vreg);
}

void RegisterAssignments::Editor::UpdateStackSlots(
    const std::unordered_map<Value, Value>& new_assignments) {
  DCHECK_EQ(assignments_->stack_slot_map_.size(), new_assignments.size());
#ifndef _NDEBUG
  for (auto const pair : new_assignments)
    DCHECK(assignments_->stack_slot_map_.count(pair.first));
#endif
  assignments_->stack_slot_map_.clear();
  assignments_->stack_slot_map_.insert(new_assignments.begin(),
                                       new_assignments.end());
}

//////////////////////////////////////////////////////////////////////
//
// RegisterAssignments
//
RegisterAssignments::RegisterAssignments()
    : block_value_map_(zone()),
      before_action_map_(zone()),
      empty_actions_(zone()),
      instruction_value_map_(zone()),
      stack_slot_map_(zone()) {
}

RegisterAssignments::~RegisterAssignments() {
}

Value RegisterAssignments::AllocationOf(BasicBlock* block, Value value) const {
  if (!value.is_virtual())
    return value;
  auto const it = block_value_map_.find(std::make_pair(block, value));
  DCHECK(it != block_value_map_.end());
  return it->second;
}

Value RegisterAssignments::AllocationOf(Instruction* instr, Value value) const {
  if (!value.is_virtual())
    return value;
  auto const it = instruction_value_map_.find(std::make_pair(instr, value));
  DCHECK(it != instruction_value_map_.end());
  return it->second;
}

const ZoneVector<Instruction*>& RegisterAssignments::BeforeActionOf(
    Instruction* instr) const {
  auto const it = before_action_map_.find(instr);
  return it == before_action_map_.end() ? empty_actions_ : it->second->actions;
}

Value RegisterAssignments::StackSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = stack_slot_map_.find(vreg);
  return it == stack_slot_map_.end() ? Value() : it->second;
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
