// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <utility>

#include "elang/lir/transforms/phi_expander.h"

#include "base/logging.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/parallel_copy_expander.h"
#include "elang/lir/transforms/register_allocation_tracker.h"
#include "elang/lir/transforms/spill_manager.h"

namespace elang {
namespace lir {

namespace {
Value AdjustSize(Value type, Value value) {
  DCHECK_EQ(type.type, value.type);
  return Value(type.type, type.size, value.kind, value.data);
}

std::array<Value, 4> IntegerTypesAndFloatTypes() {
  return {Value::Int32Type(),
          Value::Int64Type(),
          Value::Float32Type(),
          Value::Float64Type()};
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// PhiExpander
//
PhiExpander::PhiExpander(RegisterAllocationTracker* allocation_tracker,
                         SpillManager* spill_manager,
                         BasicBlock* phi_block,
                         BasicBlock* predecessor)
    : allocation_tracker_(allocation_tracker),
      phi_block_(phi_block),
      predecessor_(predecessor),
      spill_manager_(spill_manager) {
  DCHECK(!phi_block->phi_instructions().empty());
  DCHECK(predecessor->last_instruction()->is<JumpInstruction>());
}

PhiExpander::~PhiExpander() {
}

void PhiExpander::AddRegister(Value physical) {
  scratch_registers_.insert(Target::NaturalRegisterOf(physical));
}

Value PhiExpander::AllocationOf(Value value) const {
  DCHECK(!value.is_physical());
  if (!value.is_virtual())
    return value;
  auto const it = allocations_.find(value);
  DCHECK(it != allocations_.end()) << "Not found: " << value;
  return it->second;
}

// Pick one of physical phi input for scratch register.
Value PhiExpander::ChooseSpillRegisterFromInput(Value type) const {
  Value candidate;
  for (auto const input : input_registers_) {
    if (input.type != type.type || input.size != type.size)
      continue;
    auto const physical = AllocationOf(input);
    if (!physical.is_physical())
      continue;
    if (allocation_tracker_->SpillSlotFor(input).is_memory_proxy())
      return input;
    candidate = input;
  }
  return candidate;
}

// Pick one of physical phi output for scratch register.
Value PhiExpander::ChooseSpillRegisterFromLiveIn(Value type) const {
  Value candidate;
  for (auto const live : live_registers_) {
    if (live.type != type.type || live.size != type.size)
      continue;
    auto const physical = allocation_tracker_->PhysicalFor(live);
    DCHECK(physical.is_physical());
    if (allocation_tracker_->SpillSlotFor(live).is_memory_proxy())
      return live;
    candidate = live;
  }
  return candidate;
}

// Pick one of physical phi output for scratch register.
Value PhiExpander::ChooseSpillRegisterFromOutput(Value type) const {
  Value candidate;
  for (auto const output : output_registers_) {
    if (output.type != type.type || output.size != type.size)
      continue;
    auto const physical = AllocationOf(output);
    if (!physical.is_physical())
      continue;
    if (IsInput(physical))
      continue;
    if (allocation_tracker_->SpillSlotFor(output).is_memory_proxy())
      return output;
    candidate = output;
  }
  return candidate;
}

void PhiExpander::EmitReload(Value physical, Value vreg) {
  DCHECK(physical.is_physical()) << physical;
  DCHECK(vreg.is_virtual()) << vreg;
  reloads_.push_back(spill_manager_->NewReload(physical, vreg));
}

void PhiExpander::EmitSpill(Value vreg, Value physical) {
  DCHECK(physical.is_physical()) << physical;
  DCHECK(vreg.is_virtual()) << vreg;
  spills_.push_back(spill_manager_->NewSpill(vreg, physical));
}

// TODO(eval1749) We should use free output registers in different size, e.g.
// pcopy %f32, %f64 <= %r1, %r2, we can use %f64 as scratch register during
// expanding float 32.
void PhiExpander::Expand() {
  std::vector<Instruction*> copies;
  std::vector<std::pair<Value, Value>> tasks;

  for (auto const phi : phi_block_->phi_instructions()) {
    auto const output = phi->output(0);
    output_registers_.insert(output);
    auto const output_allocation =
        allocation_tracker_->AllocationOf(phi, output);
    allocations_[output] = output_allocation;
    if (output_allocation.is_physical())
      scratch_registers_.erase(Target::NaturalRegisterOf(output_allocation));

    auto const input = phi->input_of(predecessor_);
    if (!input.is_virtual()) {
      allocations_[input] = input;
      tasks.push_back(std::make_pair(output, input));
      continue;
    }

    input_registers_.insert(input);
    auto const input_allocation =
        allocation_tracker_->AllocationOf(predecessor_, input);
    if (input_allocation == output_allocation)
      continue;
    tasks.push_back(std::make_pair(output, input));
    allocations_[input] = input_allocation;
  }

  if (tasks.empty())
    return;

  // Exclude live registers after phi-instruction from scratch register list.
  for (auto const pair : allocation_tracker_->physical_map()) {
    auto const vreg = pair.first;
    if (!output_registers_.count(vreg) && !input_registers_.count(vreg))
      live_registers_.insert(vreg);
    scratch_registers_.erase(Target::NaturalRegisterOf(pair.second));
  }

  // Expand parallel copy for each type.
  for (auto const type : IntegerTypesAndFloatTypes()) {
    // Expander needs at most two scratch registers.
    for (auto count = 0; count < 2; ++count) {
      ParallelCopyExpander expander(spill_manager_->factory(), type);

      // Add task to |expander|.
      for (auto const task : tasks) {
        auto const output = task.first;
        if (output.type != type.type || output.size != type.size)
          continue;
        auto const input = task.second;
        DCHECK_EQ(output.type, input.type);
        DCHECK_EQ(output.size, input.size);
        expander.AddTask(AllocationOf(output), AllocationOf(input));
      }
      if (!expander.HasTasks())
        break;

      // Tells available scratch registers to |expander|.
      for (auto const natural : scratch_registers_) {
        if (natural.type != type.type)
          continue;
        expander.AddScratch(AdjustSize(type, natural));
      }

      // Emit copy instructions for `phi` instructions.
      auto const instructions = expander.Expand();
      if (!instructions.empty()) {
        copies.insert(copies.end(), instructions.begin(), instructions.end());
        break;
      }

      if (SpillFromInput(type) || SpillFromOutput(type))
        continue;
      SpillFromLiveIn(type);
    }
  }

  auto const last_instruction = predecessor_->last_instruction();
  for (auto const instr : spills_)
    allocation_tracker_->InsertBefore(instr, last_instruction);
  for (auto const instr : copies)
    allocation_tracker_->InsertBefore(instr, last_instruction);
  for (auto const instr : reloads_)
    allocation_tracker_->InsertBefore(instr, last_instruction);
}

bool PhiExpander::IsInput(Value physical) const {
  DCHECK(physical.is_physical());
  auto const natural = Target::NaturalRegisterOf(physical);
  for (auto const phi : phi_block_->phi_instructions()) {
    auto const input = phi->input_of(predecessor_);
    if (Target::NaturalRegisterOf(AllocationOf(input)) == natural)
      return true;
  }
  return false;
}

// Spill one of phi input register to make scratch register.
bool PhiExpander::SpillFromInput(Value type) {
  auto const victim = ChooseSpillRegisterFromInput(type);
  if (!victim.is_virtual())
    return false;

  auto const spill_slot = spill_manager_->SpillSlotFor(victim);
  if (spill_slot.is_memory_proxy()) {
    UpdateAllocationForSpill(victim, spill_slot);
    return true;
  }

  auto const new_spill_slot = spill_manager_->EnsureSpillSlot(victim);
  auto const physical = UpdateAllocationForSpill(victim, new_spill_slot);
  EmitSpill(victim, physical);
  auto const after = allocation_tracker_->PhysicalFor(victim);
  if (!after.is_physical())
    return true;
  DCHECK_EQ(physical, after);
  EmitSpill(victim, physical);
  return true;
}

// Spill one of live-in register to make scratch register.
void PhiExpander::SpillFromLiveIn(Value type) {
  auto const victim = ChooseSpillRegisterFromLiveIn(type);
  DCHECK(victim.is_virtual());
  DCHECK(!allocations_.count(victim));

  auto const physical = allocation_tracker_->PhysicalFor(victim);
  DCHECK(physical.is_physical());

  auto const spill_slot = spill_manager_->SpillSlotFor(victim);
  if (spill_slot.is_memory_proxy()) {
    EmitReload(physical, victim);
    return;
  }

  auto const new_spill_slot = spill_manager_->EnsureSpillSlot(victim);
  EmitSpill(victim, physical);
  EmitReload(physical, victim);
}

// Spill one of phi output register to make scratch register.
bool PhiExpander::SpillFromOutput(Value type) {
  auto const victim = ChooseSpillRegisterFromOutput(type);
  if (!victim.is_virtual())
    return false;
  auto const spill_slot = spill_manager_->EnsureSpillSlot(victim);
  auto const physical = UpdateAllocationForSpill(victim, spill_slot);
  EmitReload(physical, victim);
  return true;
}

Value PhiExpander::UpdateAllocationForSpill(Value vreg, Value spill_slot) {
  DCHECK(vreg.is_virtual());
  DCHECK(spill_slot.is_memory_proxy());
  auto const physical = AllocationOf(vreg);
  DCHECK(physical.is_physical()) << "Invalid AllocationOf(" << vreg << ")";
  allocations_[vreg] = spill_slot;
  scratch_registers_.insert(Target::NaturalRegisterOf(physical));
  return physical;
}

}  // namespace lir
}  // namespace elang
