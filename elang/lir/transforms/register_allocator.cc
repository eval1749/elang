// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/lir/transforms/register_allocator.h"

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/lir/analysis/use_def_list.h"
#include "elang/lir/factory.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/register_allocation.h"
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

int PreferenceOfInLeaf(const Value& reg) {
  DCHECK(reg.is_physical());
  if (Target::IsCallerSavedRegister(reg))
    return 0;
  if (Target::IsCalleeSavedRegister(reg))
    return 1;
  if (Target::GetReturn(reg) == reg)
    return 9;
  if (Target::IsParameterRegister(reg))
    return 8;
  return 2;
}

int PreferenceOfInNonLeaf(const Value& reg) {
  DCHECK(reg.is_physical());
  if (Target::IsCalleeSavedRegister(reg))
    return 0;
  if (Target::IsCallerSavedRegister(reg))
    return 1;
  if (Target::GetReturn(reg) == reg)
    return 9;
  if (Target::IsParameterRegister(reg))
    return 8;
  return 2;
}

bool CompareRegisterInLeaf(const Value& reg1, const Value reg2) {
  return PreferenceOfInLeaf(reg1) < PreferenceOfInLeaf(reg2);
}

bool CompareRegisterInNonLeaf(const Value& reg1, const Value reg2) {
  return PreferenceOfInNonLeaf(reg1) < PreferenceOfInNonLeaf(reg2);
}

bool IsLeafFunction(const Function* function) {
  auto const exit_block = function->exit_block();
  for (auto block : function->basic_blocks()) {
    for (auto instr : block->instructions()) {
      if (instr->is<CallInstruction>() &&
          *block->successors().begin() != exit_block) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocator
//
RegisterAllocator::RegisterAllocator(
    const Editor* editor,
    RegisterAllocation* register_allocation,
    const LivenessCollection<BasicBlock*, Value>& liveness,
    const RegisterUsageTracker& usage_tracker,
    StackAllocator* stack_allocator)
    : allocation_tracker_(new RegisterAllocationTracker(register_allocation)),
      editor_(editor),
      dominator_tree_(editor->BuildDominatorTree()),
      liveness_(liveness),
      stack_allocator_(stack_allocator),
      usage_tracker_(usage_tracker) {
  SortAllocatableRegisters();
}

RegisterAllocator::~RegisterAllocator() {
}

Factory* RegisterAllocator::factory() const {
  return editor_->factory();
}

Function* RegisterAllocator::function() const {
  return editor_->function();
}

const std::vector<Value>& RegisterAllocator::AllocatableRegistersFor(
    Value output) const {
  return output.is_float() ? float_registers_ : general_registers_;
}

// TODO(eval1749) Allocate physical registers to frequently used phi outputs.
void RegisterAllocator::AllocatePhis(BasicBlock* block) {
  struct InputFrequency {
    Value value;
    int count;
  };
  for (auto const phi : block->phi_instructions()) {
    std::vector<InputFrequency> candidates;
    for (auto const phi_input : phi->phi_inputs()) {
      auto const input = phi_input->value();
      if (!input.is_physical())
        continue;
      auto found = false;
      for (auto& entry : candidates) {
        if (entry.value == input) {
          ++entry.count;
          found = true;
          break;
        }
      }
      if (found)
        continue;
      candidates.push_back({input, 0});
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const InputFrequency& entry1, const InputFrequency& entry2) {
      return entry1.count > entry2.count;
    });

    auto const output = phi->output(0);
    auto allocated = false;
    for (auto const candidate : candidates) {
      if (TryAllocate(phi, output, candidate.value)) {
        allocated = true;
        break;
      }
    }
    if (allocated)
      continue;

    for (auto const physical : AllocatableRegistersFor(output)) {
      if (allocation_tracker_->TryAllocate(phi, output, physical)) {
        allocated = true;
        break;
      }
    }
    if (allocated)
      continue;

    auto const stack_slot = stack_allocator_->Allocate(output);
    DCHECK(stack_slot.is_stack());
    allocation_tracker_->TrackStackSlot(output, stack_slot);
    allocation_tracker_->SetAllocation(phi, output, stack_slot);
  }
}

// Returns farthest used virtual register or farthest used virtual register.
Value RegisterAllocator::ChooseRegisterToSpill(Instruction* instr,
                                               Value vreg) const {
  SpillVictim victim;
  SpillVictim spilled_victim;
  for (auto const it : allocation_tracker_->physical_map()) {
    auto const physical = it.second;
    if (physical.type != vreg.type)
      continue;
    auto const candidate = it.first;
    auto const next_use = usage_tracker_.NextUseAfter(candidate, instr);
    DCHECK(next_use);
    if (victim.next_use < next_use->index()) {
      victim.next_use = next_use->index();
      victim.vreg = candidate;
    }
    if (StackSlotFor(candidate).is_stack() &&
        spilled_victim.next_use < next_use->index()) {
      spilled_victim.next_use = next_use->index();
      spilled_victim.vreg = candidate;
    }
  }
  if (spilled_victim.vreg.is_virtual())
    return spilled_victim.vreg;
  return victim.vreg;
}

// Called when input operands of |instr| are processed.
Value RegisterAllocator::DidProcessInputOperands(Instruction* instr) {
  Value physical_at_0;
  auto position = 0;
  for (auto const input : instr->inputs()) {
    if (!position)
      physical_at_0 = allocation_tracker_->PhysicalFor(input);
    FreeInputIfPossible(instr, input);
    ++position;
  }
  if (!instr->is<CallInstruction>())
    return physical_at_0;

  // Save live virtual registers allocated caller saved registers.
  std::vector<Instruction*> save_actions;
  for (auto const virtual_physical : allocation_tracker_->physical_map()) {
    auto const physical = virtual_physical.second;
    if (!Target::IsCallerSavedRegister(physical))
      continue;
    auto const vreg = virtual_physical.first;
    auto const stack_slot = EnsureStackSlot(vreg);
    allocation_tracker_->TrackStackSlot(vreg, stack_slot);
    save_actions.push_back(NewSpill(stack_slot, physical));
  }
  for (auto const save_action : save_actions)
    allocation_tracker_->FreePhysical(save_action->input(0));
  allocation_tracker_->SetBeforeAction(instr, save_actions);
  return Value();
}

Value RegisterAllocator::EnsureStackSlot(Value vreg) {
  DCHECK(vreg.is_virtual());
  auto const present = StackSlotFor(vreg);
  return present.is_stack() ? present : stack_allocator_->Allocate(vreg);
}

void RegisterAllocator::FreeInputIfPossible(Instruction* instr, Value input) {
  if (!input.is_virtual())
    return;
  if (usage_tracker_.IsUsedAfter(input, instr))
    return;
  allocation_tracker_->FreeVirtual(input);
}

void RegisterAllocator::MustAllocate(Instruction* instr,
                                     Value output,
                                     Value physical) {
  DCHECK(output.is_virtual());
  DCHECK(physical.is_physical());
  auto const allocated = TryAllocate(instr, output, physical);
  DCHECK(allocated);
}

Instruction* RegisterAllocator::NewReload(Value physical, Value stack_slot) {
  DCHECK(physical.is_physical());
  DCHECK(stack_slot.is_stack());
  return factory()->NewCopyInstruction(physical, stack_slot);
}

Instruction* RegisterAllocator::NewSpill(Value stack_slot, Value physical) {
  DCHECK(physical.is_physical());
  DCHECK(stack_slot.is_stack());
  return factory()->NewCopyInstruction(stack_slot, physical);
}

// Incorporate allocation map from predecessors.
void RegisterAllocator::PopulateAllocationMap(BasicBlock* block) {
  stack_allocator_->Reset();
  for (auto const predecessor : block->predecessors()) {
    auto& allocation = allocation_tracker_->AllocationOf(predecessor);
    for (auto const number : liveness_.LivenessOf(predecessor).in()) {
      auto const input = liveness_.VariableOf(number);

      // Stack location
      auto const stack_slot = allocation.StackSlotFor(input);
      if (stack_slot.is_stack()) {
        auto const present = allocation_tracker_->StackSlotFor(input);
        if (present.is_void()) {
          allocation_tracker_->TrackStackSlot(input, stack_slot);
          stack_allocator_->AllocateAt(stack_slot);
        } else {
          DCHECK_EQ(present, stack_slot);
        }
      }

      // Physical register
      auto const physical = allocation.PhysicalFor(input);
      if (!physical.is_physical())
        continue;
      auto const present = allocation_tracker_->PhysicalFor(input);
      if (present.is_void()) {
        allocation_tracker_->TrackPhysical(input, physical);
        continue;
      }
      if (present == physical)
        continue;
      allocation_tracker_->FreePhysical(physical);
    }
  }
}

void RegisterAllocator::ProcessBlock(BasicBlock* block) {
  allocation_tracker_->StartBlock(block);
  PopulateAllocationMap(block);
  AllocatePhis(block);
  auto last_instr = static_cast<Instruction*>(nullptr);
  for (auto const instr : block->instructions()) {
    auto input_position = 0;
    for (auto const input : instr->inputs()) {
      ProcessInputOperand(instr, input, input_position);
      ++input_position;
    }

    auto const physical_at_0 = DidProcessInputOperands(instr);
    auto const previous_instr = last_instr;
    last_instr = instr;

    if (physical_at_0.is_physical()) {
      if (previous_instr && previous_instr->is<AssignInstruction>() &&
          previous_instr->output(0) == instr->input(0)) {
        // We must allocate same physical register to %tmp1 and %tmp2 for two
        // operands arithmetic operation:
        //  assign %tmp1 = %2
        //  add %tmp2 = %tmp1 %3
        //  copy %5 = %tmp2
        MustAllocate(instr, instr->output(0), physical_at_0);
        continue;
      }
      if (instr->is<CopyInstruction>()) {
        MustAllocate(instr, instr->output(0), physical_at_0);
        continue;
      }
    }

    for (auto const output : instr->outputs())
      ProcessOutputOperand(instr, output);
  }
  allocation_tracker_->EndBlock(block);
  for (auto const child_node : dominator_tree_.TreeNodeOf(block)->children())
    ProcessBlock(child_node->value());
}

void RegisterAllocator::ProcessInputOperand(Instruction* instr,
                                            Value input,
                                            int position) {
  DCHECK_GE(position, 0);
  if (!input.is_virtual())
    return;
  // Use current physical register for |input| if available.
  {
    auto const physical = allocation_tracker_->PhysicalFor(input);
    if (physical.is_physical()) {
      allocation_tracker_->SetAllocation(instr, input, physical);
      return;
    }
  }

  // Spill one of register for |input| at |instr| by emitting spill and
  // reload instr:
  //    spill %stack[i] = %physical[1]
  //    reload %physical[1] = %stack[j]
  //    use %physical[1]
  //
  // TODO(eval1749) If instr can accept memory operand at |position|,
  // we can use spilled location as operand.
  auto const victim = ChooseRegisterToSpill(instr, input);
  DCHECK(victim.is_virtual());
  auto const physical = allocation_tracker_->PhysicalFor(victim);
  auto const spill_location = EnsureStackSlot(victim);
  allocation_tracker_->FreePhysical(physical);
  allocation_tracker_->SetBeforeAction(
      instr, {
              NewSpill(spill_location, physical),
              NewReload(physical, StackSlotFor(input)),
             });
  allocation_tracker_->SetAllocation(instr, input, physical);
  allocation_tracker_->TrackStackSlot(victim, spill_location);
}

void RegisterAllocator::ProcessOutputOperand(Instruction* instr, Value output) {
  if (!output.is_virtual())
    return;
  // When next use is 'copy' instr to physical register, try to allocate
  // it, for example:
  //    add %1 = %2, %3
  //    pcopy ECX, EDX = %1, %2
  // or
  //    add %1 = %2, %3
  //    copy ECX = %1
  //    assign %4 = %2
  //    shl %5 = %4, ECX
  auto const user = usage_tracker_.NextUseAfter(output, instr);
  DCHECK(user) << "The result of " << *instr << " isn't used.";
  if (user->is<CopyInstruction>() || user->is<PCopyInstruction>()) {
    auto position = 0;
    for (auto const source : user->inputs()) {
      if (source == output) {
        if (user->output(position).is_physical() &&
            TryAllocate(instr, output, user->output(position))) {
          return;
        }
      }
      ++position;
    }
  }
  for (auto const physical : AllocatableRegistersFor(output)) {
    if (allocation_tracker_->TryAllocate(instr, output, physical))
      return;
  }

  // Spill one of register for |output| at |instr| by emitting spill
  // instr:
  //    spill %stack[i] = %physical[1]
  //    def %physical[1] = ...
  //
  // TODO(eval1749) If next use of |output| is spilled and |instr| can
  // output to memory operand, we should use memory operand rather than
  // physical
  // register.
  auto const victim = ChooseRegisterToSpill(instr, output);
  DCHECK_NE(victim, output);
  DCHECK(victim.is_virtual());
  auto const physical = allocation_tracker_->PhysicalFor(victim);
  DCHECK(physical.is_physical());
  auto const spill_location = EnsureStackSlot(victim);
  allocation_tracker_->FreePhysical(physical);
  allocation_tracker_->SetBeforeAction(instr,
                                       {
                                        NewSpill(spill_location, physical),
                                       });
  allocation_tracker_->SetAllocation(instr, output, physical);
  allocation_tracker_->TrackStackSlot(victim, spill_location);
}

// FunctionPass - This is entry point of |RegisterAllocator|. We process
// basic blocks in dominator tree.
void RegisterAllocator::Run() {
  ProcessBlock(function()->entry_block());
}

void RegisterAllocator::SortAllocatableRegisters() {
  float_registers_ = Target::AllocatableFloatRegisters();
  general_registers_ = Target::AllocatableGeneralRegisters();

  if (IsLeafFunction(function())) {
    std::sort(float_registers_.begin(), float_registers_.end(),
              CompareRegisterInLeaf);
    std::sort(general_registers_.begin(), general_registers_.end(),
              CompareRegisterInLeaf);
    return;
  }

  std::sort(float_registers_.begin(), float_registers_.end(),
            CompareRegisterInNonLeaf);
  std::sort(general_registers_.begin(), general_registers_.end(),
            CompareRegisterInNonLeaf);
}

Value RegisterAllocator::StackSlotFor(Value vreg) const {
  return allocation_tracker_->StackSlotFor(vreg);
}

bool RegisterAllocator::TryAllocate(Instruction* instr,
                                    Value vreg,
                                    Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  return allocation_tracker_->TryAllocate(instr, vreg, physical);
}

}  // namespace lir
}  // namespace elang
