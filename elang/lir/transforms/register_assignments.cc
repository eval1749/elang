// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_assignments.h"

#include "base/logging.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
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

const ZoneUnorderedMap<Value, Value>& RegisterAssignments::Editor::proxy_map()
    const {
  return assignments_->proxy_map_;
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
  DCHECK(allocation.is_physical() || allocation.is_memory_proxy());
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

void RegisterAssignments::Editor::SetSpillSlot(Value vreg, Value proxy) {
  DCHECK(vreg.is_virtual());
  DCHECK(proxy.is_memory_proxy());
  DCHECK(!assignments_->proxy_map_.count(vreg));
  assignments_->proxy_map_[vreg] = proxy;
}

Value RegisterAssignments::Editor::SpillSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  return assignments_->SpillSlotFor(vreg);
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
      proxy_map_(zone()) {
}

RegisterAssignments::~RegisterAssignments() {
}

Value RegisterAssignments::AllocationOf(BasicBlock* block, Value value) const {
  if (!value.is_virtual())
    return value;
  auto const it = block_value_map_.find(std::make_pair(block, value));
  DCHECK(it != block_value_map_.end()) << value << " isn't found in " << *block;
  return it->second;
}

Value RegisterAssignments::AllocationOf(Instruction* instr, Value value) const {
  if (!value.is_virtual())
    return value;
  auto const it = instruction_value_map_.find(std::make_pair(instr, value));
  DCHECK(it != instruction_value_map_.end()) << "No allocation for " << *instr;
  return it->second;
}

const ZoneVector<Instruction*>& RegisterAssignments::BeforeActionOf(
    Instruction* instr) const {
  auto const it = before_action_map_.find(instr);
  return it == before_action_map_.end() ? empty_actions_ : it->second->actions;
}

Value RegisterAssignments::SpillSlotFor(Value vreg) const {
  DCHECK(vreg.is_virtual());
  auto const it = proxy_map_.find(vreg);
  return it == proxy_map_.end() ? Value::Void() : it->second;
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
