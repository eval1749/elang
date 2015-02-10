// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/register_allocator.h"

#include "base/logging.h"
#include "elang/lir/analysis/use_def_list.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
int RoundUp(int value, int alignment) {
  return (value + alignment - 1) / alignment * alignment;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocator::LocalAllocationMap
//
class RegisterAllocator::LocalAllocationMap final {
 public:
  LocalAllocationMap() = default;
  ~LocalAllocationMap() = default;

  Value AllocationFor(Value input) const;
  void Free(Value input);
  void Register(Value output, Value allocated);
  Value SpillLocationFor(Value input) const;

 private:
  friend class LocalAllocator;

  std::unordered_map<Value, Value> register_map_;
  std::unordered_map<Value, Value> stack_map_;

  DISALLOW_COPY_AND_ASSIGN(LocalAllocationMap);
};

Value RegisterAllocator::LocalAllocationMap::AllocationFor(Value input) const {
  DCHECK(input.is_virtual());
  auto const it = register_map_.find(input);
  DCHECK(it != register_map_.end());
  return it->second;
}

Value RegisterAllocator::LocalAllocationMap::Register(Value output,
                                                      Value allocated) {
  DCHECK(output.is_virtual());
  if (allocated.is_physical()) {
    physical_map_[output] = allocated;
    return;
  }
  if (allocated.is_stack()) {
    stack_map_[output] = allocated;
    return;
  }
  NOTREACHD() << "Unexpected allocated value: " << allocated;
}

Value RegisterAllocator::LocalAllocationMap::SpillLocationFor(
    Value input) const {
  DCHECK(input.is_virtual());
  auto const it = stack_map_.find(input);
  return it == register_map_.end() ? it->second : Value();
}

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocator::LocalAllocator
//
class RegisterAllocator::LocalAllocator final {
 public:
  LocalAllocator();
  ~LocalAllocator() = default;

  Value AllocationFor(Value input) const;
  Value Allocate(Instruction* instruction, Value output);
  void EndBlock(BasicBlock* block);
  void Free(Value value);
  void Spill(Value physical);
  void SpillFor(Instruction* instruction, Value output);
  void StartBlock(BasicBlock* block, LocalAllocationMap* local_map);
  Value TryAllocate(Value output);
  void WillCall(Instruction* instruction);

 private:
  std::unordered_map<BasicBlock*, LocalAllocationMap*> block_map_;
  const std::vector<Value>& float_registers_;
  const std::vector<Value>& general_registers_;
  LocalAllocationMap* local_map_;

  // physical register to virtual register map.
  std::unordered_map<Value, Value> physical_map_;

  DISALLOW_COPY_AND_ASSIGN(LocalAllocator);
};

RegisterAllocator::LocalAllocator::LocalAllocator()
    : float_registers_(Target::AllocatableFloatRegisters()),
      general_registers_(Target::AllocatableGeneralRegisters())),
      local_map_(nullptr) {
}

RegisterAllocator::LocalAllocator::~LocalAllocator() {
}

Value RegisterAllocator::LocalAllocator::AllocationFor(Value input) const {
  return local_map_->AllocationFor(input);
}

Value RegisterAllocator::LocalAllocator::Allocate(Instruction* instruction,
                                                  Value output) {
  if (!output.is_virtual())
    return;
  auto const maybe_physical = TryAllocate(output);
  if (maybe_physical.is_physical())
    return maybe_physical;
  return SpillFor(instruction, output);
}

const std::vector<Value>&
RegisterAllocator::LocalAllocator::AllocatableRegistersFor(Value output) {
  return output->is_float() ? float_registers_ : general_registers_;
}

void RegisterAllocator::LocalAllocator::EndBlock(BasicBlock* block) const {
  auto const it = block_map_.find(block);
  DCHECK(it != block_map_.end());
  block_map_.erase(it);
}

void RegisterAllocator::LocalAllocator::Free(Value value) {
  DCHECK(value->is_virtual());
  auto const physical = local_map_->AllocationFor(value);
  DCHECK(physical->is_physical());
  auto const it = physical_map_.find(physical);
  DCHECK(it != physical_map_.end());
  physical_map_.erase(it);
  local_map_->Free(value);
}

void RegisterAllocator::LocalAllocator::Spill(Value physical) {
  DCHECK(physical->is_physical());
  auto const it = physical_map_.find(physical);
  DCHECK(it != physical_map_.end());
  auto const mapped = it->second;
  auto const spill_location = SpillLocationFor(mapped);
  if (!spill_location.is_stack())
    local_map_->Register(mapped, stack_allocator_->Allocate(mapped));
  physical_map_.erase(it);
}

// Spill most lately used.
Value RegisterAllocator::LocalAllocator::SpillFor(Instruction* instruction,
                                                  Value output) {
  Value candidate;
  Value physical;
  auto candidate_next_use = 0;
  for (auto const physical_virtual : physical_map_) {
    if (physical_virtual.first.type != output.type)
      continue;
    auto const next_use = NextUseOf(value);
    if (candidate.is_virtual() && candidate_next_use > next_use)
      continue;
    physical = physical_virtual.first;
    candidate = physical_virtual.second;
    candidate_next_use = next_use;
  }
  DCHECK(candidate.is_virtual());
  Spill(candidate);
  return Value(physical.type, output.size, physical.kind, physical.data);
}

void RegisterAllocator::LocalAllocator::StartBlock(
    BasicBlock* block,
    LocalAllocationMap* local_map) {
  DCHECK(!block_map_.count(block));
  block_map_[block] local_map;
}

Value RegisterAllocator::LocalAllocator::TryAllocate(Value output) {
  DCHEC(output->is_virtual());
  for (auto const candidate : AllocatableRegistersFor(output)) {
    if (!physical_map_.count(candidate)) {
      physical_map_[candidate] = output;
      local_map_->Register(output, candidate);
      return Value(candidate.type, output.size, candidate.kind, candidate.data);
    }
  }
  return Value();
}

// Spill caller save registers.
Value RegisterAllocator::LocalAllocator::WillCall(Instruction* instr) {
  DCHECK(instr->is<CallInstruction>());
  for (auto const physical_virtual : physical_map_) {
    if (!Target::IsCallerSaveRegsiter(physical_virtual.first))
      continue;
    Spill(physical_virtual.first);
  }
}

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocator
//
RegisterAllocator::RegisterAllocator(Factory* factory,
                                     Function* function,
                                     RegisterAllocationMap* allocation_map,
                                     const RegisterUsageTracker& usage_tracker)
    : FunctionPass(factory, function),
      allocation_map_(allocation_map),
      dominator_tree_(editor()->ComputeDominatorTree()),
      local_allocator_(new LocalAllocator()),
      usage_tracker_(usage_tracker) {
}

RegisterAllocator::~RegisterAllocator() {
}

// Pass
base::StringPiece RegisterAllocator::name() const {
  return "register_allocator";
}

int RegisterAllocator::NextUseOf(Instruction* instr, Value input) const {
  DCHECK(input->is_virtual());
  return usage_tracker_->NextUseOf(instr, input);
}

RegisterUsageTracker::NextUseOf(Instruction* instr, Value input) const {
  DCHECK(input->is_virtual());
  auto next_use = instr->index();
  for (auto const user : usage_tracker_.UsersOf(input)) {
    // TODO(eval1749) Check |user| post-dominate |instr|.
    if (next_use > user.index())
      next_use = user.index();
  }
  return next_use == instr->index() ? -1 : next_use;
}

// FunctionPass
void RegisterAllocator::RunOnFunction() {
  ProcessBlock(function()->entry_block());
}

void RegisterAllocator::ProcessBlock(BasicBlock* block) {
  LocalAllocationMap local_map(block);
  local_allocator_->StartBlock(block, &local_map);
  block_allocation_map_[block] = &local_map;
  for (auto const instr : block->instructions()) {
    auto input_position = 0;
    for (auto const input : instr->inputs()) {
      ProcessInputOperand(instr, input, input_position);
      ++input_position;
    }
    if (instr->is<CallInstruciton>())
      local_allocator_->WillCall(instr);
    for (auto const output : instr->outputs())
      ProcessOutputOperand(instr, output);
  }
  // TODO(eval1749) Process phi in successors
  for (auto const child_node : dominator_tree_->TreeNodeOf(block)->children())
    ProcessBlock(child_node->value());
  local_allocator->EndBlock(block);
}

void RegisterAllocator::ProcessInputOperand(Instruction* instr,
                                            Value input,
                                            int position) {
  DCHECK_GE(position, 0);
  if (!input.is_virtual())
    return;
  // TODO(eval1749) If |input| is spilled and instruction can accept memory
  // operand at |position|, we can use spilled location as operand.
  auto const physical = local_allocator_->AllocationFor(input);
  DCHECK(physical->is_physical());
  allocator_map_->Set(instr, input, physical);
  auto const next_use = NextUseOf(instr, input);
  if (next_use >= 0)
    return;
  local_allocator_->Free(input);
}

void RegisterAllocator::ProcessOutputOperand(Instruction* instr, Value output) {
  if (!output.is_virtual())
    return;
  auto const physical = local_allocator->Allocate(instr, output);
  DCHECK(!physical.is_physical());
  allocation_map_->Set(instr, output, physical);
}

}  // namespace lir
}  // namespace elang
