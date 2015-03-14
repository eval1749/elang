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
#include "elang/lir/transforms/phi_expander.h"
#include "elang/lir/transforms/parallel_copy_expander.h"
#include "elang/lir/transforms/register_allocation_tracker.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/transforms/register_usage_tracker.h"
#include "elang/lir/transforms/spill_manager.h"
#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/value.h"

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
RegisterAllocator::RegisterAllocator(Editor* editor,
                                     RegisterAssignments* register_assignments,
                                     StackAssignments* stack_assignments)
    : allocation_tracker_(new RegisterAllocationTracker(register_assignments)),
      dominator_tree_(editor->BuildDominatorTree()),
      editor_(editor),
      liveness_(editor->AnalyzeLiveness()),
      stack_allocator_(new StackAllocator(editor, stack_assignments)),
      usage_tracker_(new RegisterUsageTracker(editor)),
      spill_manager_(new SpillManager(factory(),
                                      allocation_tracker_.get(),
                                      stack_allocator_.get(),
                                      usage_tracker_.get())) {
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

Value RegisterAllocator::AssignedPhysicalFor(Instruction* instr) {
  auto const previous = instr->previous();
  if (!previous || !previous->is<AssignInstruction>())
    return Value();
  auto const output = previous->output(0);
  auto const physical = allocation_tracker_->AllocationOf(previous, output);
  DCHECK(physical.is_physical())
      << *previous << " must output to physical register, but " << physical;
  DCHECK(allocation_tracker_->PhysicalFor(output).is_void())
      << *previous << " output is must be free, but "
      << allocation_tracker_->PhysicalFor(output);
  return physical;
}

// Returns farthest used virtual register which types is |type| or farthest
// used virtual register.
Value RegisterAllocator::ChooseRegisterToSpill(Instruction* instr,
                                               Value type) const {
  return spill_manager_->ChooseRegisterToSpill(type, instr);
}

Value RegisterAllocator::EnsureSpillSlot(Value vreg) {
  return spill_manager_->EnsureSpillSlot(vreg);
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

bool RegisterAllocator::HasBackEdge(BasicBlock* block) const {
  for (auto const predecessor : block->predecessors()) {
    if (IsBackEdge(predecessor, block))
      return true;
  }
  return false;
}

bool RegisterAllocator::IsBackEdge(BasicBlock* from, BasicBlock* to) const {
  auto& rpo_list = editor_->ReversePostOrderList();
  return rpo_list.position_of(from) >= rpo_list.position_of(to);
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

Instruction* RegisterAllocator::NewSpill(Value spill_slot, Value physical) {
  DCHECK(physical.is_physical());
  DCHECK(spill_slot.is_memory_proxy());
  return factory()->NewCopyInstruction(spill_slot, physical);
}

Value RegisterAllocator::PhysicalFor(Value value) const {
  if (!value.is_virtual())
    return value;
  return allocation_tracker_->PhysicalFor(value);
}

// Incorporate allocations for live-in from predecessors.
void RegisterAllocator::PopulateAllocationMap(BasicBlock* block) {
  DCHECK(allocation_tracker_->physical_map().empty());

  // Populate stack slots by LiveIn(block)
  stack_allocator_->Reset();
  for (auto const number : liveness_.LivenessOf(block).in()) {
    auto const input = liveness_.VariableOf(number);
    // Stack location
    auto const spill_slot = SpillSlotFor(input);
    if (!spill_slot.is_memory_proxy())
      continue;
    stack_allocator_->Reallocate(input, spill_slot);
  }

  // Populate virtual-physical-register tracker.
  for (auto const predecessor : block->predecessors()) {
    if (IsBackEdge(predecessor, block)) {
      // We've not yet processed |predecessor|. We assume allocations at end of
      // |predecessor| are as same as other predecessors.
      // |ProcessPredecessors()| will realize this assumption.
      continue;
    }
    for (auto const number : liveness_.LivenessOf(block).in()) {
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
  ProcessPhiOutputOperands(block);
  for (auto const instr : block->instructions()) {
    ProcessInputOperands(instr);
    instr->Accept(this);
  }
  allocation_tracker_->EndBlock(block);
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

  DCHECK(SpillSlotFor(input).is_spill_slot())
      << input << " doesn't have spill slot at " << *instr;

  if (instr->is<ArrayLoadInstruction>() && position == 0) {
    // Since first operand of |ArrayInstruction| is used for GC map, we don't
    // need to allocate physical register for it.
    return;
  }

  // Spill one of register for |input| at |instr| by emitting spill and
  // reload instr:
  //    spill %stack[i] = %physical[1]
  //    reload %physical[1] = %stack[j]
  //    use %physical[1]
  //
  // TODO(eval1749) If |instr| can accept memory operand at |position|,
  // we can use spilled location as operand.
  auto const victim = ChooseRegisterToSpill(instr, input);
  DCHECK_NE(victim, input);
  auto const physical = Spill(instr, victim);
  auto const reload = spill_manager_->NewReload(physical, input);
  allocation_tracker_->InsertBefore(reload, instr);
  allocation_tracker_->SetAllocation(instr, input, physical);
}

void RegisterAllocator::ProcessInputOperands(Instruction* instr) {
  if (instr->is<UseInstruction>())
    return;
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
  if (auto const user = usage_tracker_->NextUseAfter(output, instr)) {
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

void RegisterAllocator::ProcessPhiInputOperands(BasicBlock* block,
                                                BasicBlock* predecessor) {
  DCHECK(predecessor->last_instruction()->is<JumpInstruction>());
  DCHECK_EQ(predecessor->successors().size(), 1u);
  if (block->phi_instructions().empty())
    return;

  PhiExpander expander(allocation_tracker_.get(),
                       spill_manager_.get(),
                       block,
                       predecessor);

  // Tells available registers to |expander|.
  for (auto const natural : float_registers_)
    expander.AddRegister(natural);
  for (auto const natural : general_registers_)
    expander.AddRegister(natural);

  expander.Expand();
}

// TODO(eval1749) Allocate physical registers to frequently used phi outputs.
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

    auto const spill_slot = EnsureSpillSlot(output);
    DCHECK(spill_slot.is_memory_proxy());
    allocation_tracker_->SetAllocation(phi, output, spill_slot);
  }
}

// Shuffle registers in predecessors if |block| has phi instructions or
// has back edge.
void RegisterAllocator::ProcessPredecessors(BasicBlock* block) {
  if (block->phi_instructions().empty() && !HasBackEdge(block))
    return;
  allocation_tracker_->StartBlock(block);
  PopulateAllocationMap(block);

  // Adjust live-in registers
  for (auto const predecessor : block->predecessors()) {
    DCHECK(predecessor->last_instruction()->is<JumpInstruction>());
    for (auto const pair : allocation_tracker_->physical_map()) {
      auto const vreg = pair.first;
      auto const physical = pair.second;
      auto assignment = allocation_tracker_->AllocationOf(predecessor, vreg);
      if (assignment == physical)
        continue;
      allocation_tracker_->InsertBefore(
          factory()->NewCopyInstruction(physical, assignment),
          predecessor->last_instruction());
    }
    ProcessPhiInputOperands(block, predecessor);
  }
  allocation_tracker_->EndBlock(block);
}

// FunctionPass - This is entry point of |RegisterAllocator|.
void RegisterAllocator::Run() {
  for (auto const block : editor_->ReversePostOrderList())
    ProcessBlock(block);

  for (auto const block : function()->basic_blocks())
    ProcessPredecessors(block);
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
  DCHECK(victim.is_virtual()) << "Failed to choose spill victim for " << *instr;
  auto const physical = PhysicalFor(victim);
  DCHECK(physical.is_physical());
  allocation_tracker_->FreePhysical(physical);
  allocation_tracker_->InsertBefore(NewSpill(victim, physical), instr);
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
  if (instr->CountOutputs() == 1 && instr->CountInputs() >= 1) {
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
  stack_allocator_->TrackCall(instr);
  std::vector<Instruction*> save_actions;
  for (auto const virtual_physical : allocation_tracker_->physical_map()) {
    auto const physical = virtual_physical.second;
    if (!Target::IsCallerSavedRegister(physical))
      continue;
    auto const vreg = virtual_physical.first;
    save_actions.push_back(NewSpill(vreg, physical));
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
    auto const next_use = usage_tracker_->NextUseAfter(output, instr);
    if (next_use && !usage_tracker_->IsUsedAfter(output, next_use)) {
      MustAllocate(instr, output, physical);
      return;
    }
  }
  if (input.is_parameter())
    stack_allocator_->Assign(output, input);
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
      if (input.is_parameter())
        stack_allocator_->Assign(output, input);
      pairs.push_back(std::make_pair(output, input));
    }
    ExpandParallelCopy(pairs, instr);
  }
}

void RegisterAllocator::VisitUse(UseInstruction* instr) {
  DCHECK(!AllocationOf(instr->input(0)).is_void());
}
}  // namespace lir
}  // namespace elang
