// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <array>
#include <unordered_set>

#include "elang/lir/transforms/register_allocator.h"

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/base/ordered_list.h"
#include "elang/lir/analysis/conflict_map.h"
#include "elang/lir/analysis/use_def_list.h"
#include "elang/lir/factory.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/parallel_copy_expander.h"
#include "elang/lir/transforms/register_allocation_tracker.h"
#include "elang/lir/transforms/register_assignments.h"
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

Value AdjustSize(Value type, Value value) {
  DCHECK_EQ(type.type, value.type);
  return Value(type.type, type.size, value.kind, value.data);
}

Value AssignedPhysicalFor(Instruction* instr) {
  auto const previous = instr;
  if (!previous || !previous->is<AssignInstruction>())
    return Value();
  return previous->output(0);
}

std::array<Value, 4> IntegerTypesAndFloatTypes() {
  return {Value::Int32Type(),
          Value::Int64Type(),
          Value::Float32Type(),
          Value::Float64Type()};
}

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
RegisterAllocator::RegisterAllocator(const Editor* editor,
                                     RegisterAssignments* register_assignments,
                                     StackAssignments* stack_assignments)
    : allocation_tracker_(new RegisterAllocationTracker(register_assignments)),
      dominator_tree_(editor->BuildDominatorTree()),
      editor_(editor),
      liveness_(editor->AnalyzeLiveness()),
      stack_allocator_(new StackAllocator(editor, stack_assignments)),
      usage_tracker_(new RegisterUsageTracker(editor)) {
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

Value RegisterAllocator::AllocationOf(Value value) const {
  return value.is_virtual() ? allocation_tracker_->AllocationOf(value) : value;
}

// Returns farthest used virtual register which types is |type| or farthest
// used virtual register.
Value RegisterAllocator::ChooseRegisterToSpill(Instruction* instr,
                                               Value type) const {
  SpillVictim victim;
  SpillVictim spilled_victim;
  for (auto const it : allocation_tracker_->physical_map()) {
    auto const physical = it.second;
    if (physical.type != type.type)
      continue;
    auto const candidate = it.first;
    auto const next_use = usage_tracker_->NextUseAfter(candidate, instr);
    DCHECK(next_use);
    if (victim.next_use < next_use->index()) {
      victim.next_use = next_use->index();
      victim.vreg = candidate;
    }
    if (SpillSlotFor(candidate).is_memory_slot() &&
        spilled_victim.next_use < next_use->index()) {
      spilled_victim.next_use = next_use->index();
      spilled_victim.vreg = candidate;
    }
  }
  if (spilled_victim.vreg.is_virtual())
    return spilled_victim.vreg;
  return victim.vreg;
}

Value RegisterAllocator::EnsureSpillSlot(Value vreg) {
  DCHECK(vreg.is_virtual());
  auto const present = SpillSlotFor(vreg);
  return present.is_memory_slot() ? present : stack_allocator_->Allocate(vreg);
}

void RegisterAllocator::ExpandParallelCopy(const std::vector<ValuePair>& pairs,
                                           Instruction* ref_instr) {
  if (pairs.empty())
    return;
  auto const type = pairs.front().first;
  std::unordered_set<Value> live_registers;
  for (auto const pair : allocation_tracker_->physical_map()) {
    if (pair.first.type != type.type)
      continue;
    if (pair.first.size != type.size)
      continue;
    live_registers.insert(pair.second);
  }

  // Expander needs at most two scratch registers.
  for (auto count = 0; count < 2; ++count) {
    ParallelCopyExpander expander(factory(), type);
    for (auto const pair : pairs) {
      auto const output = AllocationOf(pair.first);
      if (output.is_physical())
        live_registers.insert(output);
      auto const input = AllocationOf(pair.second);
      if (input.is_physical())
        live_registers.insert(input);
      expander.AddTask(output, input);
    }
    if (!expander.HasTasks())
      return;
    // Tells available scratch registers to Expander.
    for (auto const natural : AllocatableRegistersFor(type)) {
      auto const value = AdjustSize(type, natural);
      if (live_registers.count(value))
        continue;
      expander.AddScratch(value);
    }
    auto const expandeds = expander.Expand();
    if (!expandeds.empty()) {
      for (auto const expanded : expandeds)
        allocation_tracker_->InsertBefore(expanded, ref_instr);
      return;
    }
    // Since expander requires scratch register, spill one of live registers.
    Spill(ref_instr, ChooseRegisterToSpill(ref_instr, type));
  }
  NOTREACHED() << "Failed to expand pcopy instruction: " << *ref_instr;
}

void RegisterAllocator::FreeInputOperandsIfNotUsed(Instruction* instr) {
  for (auto const input : instr->inputs()) {
    if (!input.is_virtual())
      continue;
    if (usage_tracker_->IsUsedAfter(input, instr))
      continue;
    allocation_tracker_->FreeVirtual(input);
    auto const spill_slot = SpillSlotFor(input);
    if (spill_slot.is_void())
      continue;
    stack_allocator_->Free(input);
  }
}

void RegisterAllocator::MustAllocate(Instruction* instr,
                                     Value output,
                                     Value physical) {
  DCHECK(output.is_virtual());
  DCHECK(physical.is_physical());
  auto const allocated = TryAllocate(instr, output, physical);
  DCHECK(allocated) << physical << " is allocated to "
                    << allocation_tracker_->VirtualFor(physical) << " not to "
                    << output;
}

Instruction* RegisterAllocator::NewReload(Value physical, Value spill_slot) {
  DCHECK(physical.is_physical());
  DCHECK(spill_slot.is_memory_slot());
  return factory()->NewCopyInstruction(physical, spill_slot);
}

Instruction* RegisterAllocator::NewSpill(Value spill_slot, Value physical) {
  DCHECK(physical.is_physical());
  DCHECK(spill_slot.is_memory_slot());
  return factory()->NewCopyInstruction(spill_slot, physical);
}

Value RegisterAllocator::PhysicalFor(Value value) const {
  if (!value.is_virtual())
    return value;
  return allocation_tracker_->PhysicalFor(value);
}

// Incorporate allocation map from predecessors.
void RegisterAllocator::PopulateAllocationMap(BasicBlock* block) {
  // Populate stack slots by LiveIn(block)
  stack_allocator_->Reset();
  for (auto const number : liveness_.LivenessOf(block).in()) {
    auto const input = liveness_.VariableOf(number);
    // Stack location
    auto const spill_slot = SpillSlotFor(input);
    if (!spill_slot.is_memory_slot())
      continue;
    allocation_tracker_->TrackSpillSlot(input, spill_slot);
    stack_allocator_->Reallocate(input, spill_slot);
  }

  auto const& rpo_list = editor_->ReversePostOrderList();
  auto const rpo_number = rpo_list.position_of(block);
  for (auto const predecessor : block->predecessors()) {
    if (rpo_list.position_of(predecessor) >= rpo_number) {
      // We've not yet processed |predecessor|. So, we can't determine live
      // register allocations.
      return;
    }
  }

  // Populate virtual-physical-register tracker.
  for (auto const predecessor : block->predecessors()) {
    for (auto const number : liveness_.LivenessOf(predecessor).in()) {
      auto const input = liveness_.VariableOf(number);
      auto const physical =
          allocation_tracker_->AllocationOf(predecessor, input);
      if (!physical.is_physical())
        continue;
      auto const present = PhysicalFor(input);
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
  ProcessPhiInstructions(block);
  for (auto const instr : block->instructions()) {
    ProcessInputOperands(instr);
    instr->Accept(this);
  }
  allocation_tracker_->EndBlock(block);
  // TODO(eval1749) We should visit dominator children in RPO order.
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
    auto const physical = PhysicalFor(input);
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
  DCHECK_NE(victim, input);
  auto const physical = Spill(instr, victim);
  allocation_tracker_->InsertBefore(NewReload(physical, SpillSlotFor(input)),
                                    instr);
  allocation_tracker_->SetAllocation(instr, input, physical);
}

void RegisterAllocator::ProcessInputOperands(Instruction* instr) {
  auto input_position = 0;
  for (auto const input : instr->inputs()) {
    ProcessInputOperand(instr, input, input_position);
    ++input_position;
  }
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
  auto const user = usage_tracker_->NextUseAfter(output, instr);
  DCHECK(user) << "The result of " << output << " of " << *instr
               << " isn't used.";
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
  for (auto const natural : AllocatableRegistersFor(output)) {
    auto const physical = AdjustSize(output, natural);
    if (TryAllocate(instr, output, physical))
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
  allocation_tracker_->SetAllocation(instr, output, Spill(instr, victim));
}

void RegisterAllocator::ProcessOutputOperands(Instruction* instr) {
  for (auto const output : instr->outputs())
    ProcessOutputOperand(instr, output);
}

// TODO(eval1749) Allocate physical registers to frequently used phi outputs.
void RegisterAllocator::ProcessPhiInstructions(BasicBlock* block) {
  if (block->phi_instructions().empty())
    return;

  ProcessPhiOutputOperands(block);

  // TODO(eval1749) We should use free output registers in different size, e.g.
  // pcopy %f32, %f64 <= %r1, %r2, we can use %f64 as scratch register durign
  // expanding float 32.
  // Insert pcopy expanded into predecessors.
  for (auto const type : IntegerTypesAndFloatTypes()) {
    for (auto const predecessor : block->predecessors()) {
      DCHECK_EQ(predecessor->successors().size(), 1u);
      std::vector<ValuePair> pairs;
      for (auto const phi : block->phi_instructions()) {
        auto const output = phi->output(0);
        if (output.type != type.type || output.size != type.size)
          continue;
        auto const input = phi->FindPhiInputFor(predecessor)->value();
        DCHECK_EQ(output.type, input.type);
        DCHECK_EQ(output.size, input.size);
        pairs.push_back(std::make_pair(output, input));
      }
      auto const last = predecessor->last_instruction();
      DCHECK(last->is<JumpInstruction>());
      ExpandParallelCopy(pairs, last);
    }
  }
}

void RegisterAllocator::ProcessPhiOutputOperands(BasicBlock* block) {
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

    for (auto const natural : AllocatableRegistersFor(output)) {
      auto const physical = AdjustSize(output, natural);
      if (TryAllocate(phi, output, physical)) {
        allocated = true;
        break;
      }
    }
    if (allocated)
      continue;

    auto const spill_slot = stack_allocator_->Allocate(output);
    DCHECK(spill_slot.is_memory_slot());
    allocation_tracker_->TrackSpillSlot(output, spill_slot);
    allocation_tracker_->SetAllocation(phi, output, spill_slot);
  }
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

Value RegisterAllocator::Spill(Instruction* instr, Value victim) {
  DCHECK(victim.is_virtual());
  auto const physical = PhysicalFor(victim);
  DCHECK(physical.is_physical());
  auto const spill_slot = EnsureSpillSlot(victim);
  allocation_tracker_->FreePhysical(physical);
  allocation_tracker_->TrackSpillSlot(victim, spill_slot);
  allocation_tracker_->InsertBefore(NewSpill(spill_slot, physical), instr);
  return physical;
}

Value RegisterAllocator::SpillSlotFor(Value vreg) const {
  return allocation_tracker_->SpillSlotFor(vreg);
}

bool RegisterAllocator::TryAllocate(Instruction* instr,
                                    Value vreg,
                                    Value physical) {
  DCHECK(vreg.is_virtual());
  DCHECK(physical.is_physical());
  if (!allocation_tracker_->TryAllocate(instr, vreg, physical))
    return false;
  if (!Target::IsCalleeSavedRegister(physical))
    return true;
  stack_allocator_->AllocateForPreserving(physical);
  return true;
}

// InstructionVisitor
void RegisterAllocator::DoDefaultVisit(Instruction* instr) {
  FreeInputOperandsIfNotUsed(instr);
  if (instr->CountOutputs() == 1 && instr->CountOutputs() >= 1) {
    auto const physical = AssignedPhysicalFor(instr);
    if (physical.is_physical()) {
      // We must allocate same physical register to %tmp1 and %tmp2 for two
      // operands arithmetic operation:
      //  assign %tmp1 = %2
      //  add %tmp2 = %tmp1 %3
      //  copy %5 = %tmp2
      MustAllocate(instr, instr->output(0), physical);
      return;
    }
  }

  ProcessOutputOperands(instr);
}

// Save live physical registers allocated caller saved registers.
void RegisterAllocator::VisitCall(CallInstruction* instr) {
  // Track number of arguments
  if (auto const previous = instr->previous()) {
    if (previous->is<CopyInstruction>() || previous->is<PCopyInstruction>())
      stack_allocator_->TrackNumberOfArguments(previous->CountInputs());
  }
  std::vector<Instruction*> save_actions;
  for (auto const virtual_physical : allocation_tracker_->physical_map()) {
    auto const physical = virtual_physical.second;
    if (!Target::IsCallerSavedRegister(physical))
      continue;
    auto const vreg = virtual_physical.first;
    auto const spill_slot = EnsureSpillSlot(vreg);
    allocation_tracker_->TrackSpillSlot(vreg, spill_slot);
    save_actions.push_back(NewSpill(spill_slot, physical));
  }
  for (auto const save_action : save_actions) {
    allocation_tracker_->FreePhysical(save_action->input(0));
    allocation_tracker_->InsertBefore(save_action, instr);
  }
}

// Allocate output and input to same physical register if possible.
void RegisterAllocator::VisitCopy(CopyInstruction* instr) {
  auto const input = instr->input(0);
  auto const physical = PhysicalFor(input);
  FreeInputOperandsIfNotUsed(instr);
  auto const output = instr->output(0);
  if (output.is_physical())
    return;
  DCHECK(output.is_virtual());
  if (physical.is_physical()) {
    MustAllocate(instr, output, physical);
    return;
  }
  if (input.is_parameter()) {
    stack_allocator_->Assign(output, input);
    allocation_tracker_->TrackSpillSlot(output, input);
  }
  ProcessOutputOperand(instr, output);
}

void RegisterAllocator::VisitPCopy(PCopyInstruction* instr) {
  FreeInputOperandsIfNotUsed(instr);
  ProcessOutputOperands(instr);
  // TODO(eval1749) We should use free output registers in different size, e.g.
  // pcopy %f32, %f64 <= %r1, %r2, we can use %f64 as scratch register during
  // expanding float 32.
  for (auto const type : IntegerTypesAndFloatTypes()) {
    std::vector<ValuePair> pairs;
    auto inputs = instr->inputs().begin();
    for (auto const output : instr->outputs()) {
      auto const input = *inputs;
      ++inputs;
      if (output.size != type.size || output.type != type.type)
        continue;
      DCHECK_EQ(output.size, input.size);
      DCHECK_EQ(output.type, input.type);
      if (input.is_parameter()) {
        stack_allocator_->Assign(output, input);
        allocation_tracker_->TrackSpillSlot(output, input);
      }
      pairs.push_back(std::make_pair(output, input));
    }
    ExpandParallelCopy(pairs, instr);
  }
}

}  // namespace lir
}  // namespace elang
