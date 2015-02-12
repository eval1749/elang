// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {

template <typename Graph>
class DominatorTree;

template <typename BasicBlock, typename Value>
class LivenessCollection;

namespace lir {

class BasicBlock;
class Editor;
class Factory;
class Function;
class Instruction;
class LocalAllocation;
class RegisterAllocation;
class RegisterAllocationTracker;
class RegisterUsageTracker;
class StackAllocator;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocator makes
//  - mapping of virtual register input operand in instruction to physical
//    register or stack location.
//  - mapping of virtual register output operand of instruction to physical
//    register or stack location.
//  - mapping of instruction to instructions for spilling, reloading and
//    saving callee saved registers.
//  and store them in |RegisterAllocation| instance via
//  |RegisterAllocationTracker|.
//
class ELANG_LIR_EXPORT RegisterAllocator final {
 public:
  RegisterAllocator(const Editor* editor,
                    RegisterAllocation* register_allocation,
                    const LivenessCollection<BasicBlock*, Value>& liveness,
                    const RegisterUsageTracker& usage_tracker,
                    StackAllocator* stack_allocator);
  ~RegisterAllocator();

  // The entry point
  void Run();

 private:
  Factory* factory() const;
  Function* function() const;

  const std::vector<Value>& AllocatableRegistersFor(Value output) const;
  void AllocatePhis(BasicBlock* block);
  // Called when input operands of |instruction| are processed.
  Value DidProcessInputOperands(Instruction* instruction);
  Value ChooseRegisterToSpill(Instruction* instruction, Value vreg) const;
  Value EnsureStackSlot(Value vreg);
  void FreeInputIfNotUsed(Instruction* instruction, Value input);
  void MustAllocate(Instruction* instruction, Value output, Value physical);
  Instruction* NewReload(Value physical, Value stack_slot);
  Instruction* NewSpill(Value stack_slot, Value physical);
  void PopulateAllocationMap(BasicBlock* block);
  void ProcessBlock(BasicBlock* block);
  void ProcessInputOperand(Instruction* instruction, Value input, int position);
  void ProcessOutputOperand(Instruction* instruction, Value output);
  void SortAllocatableRegisters();

  // Returns true if |output| is allocated to |physical|, or returns false.
  bool TryAllocate(Instruction* instruction, Value output, Value physical);

  Value StackSlotFor(Value virtual_register) const;

  std::unique_ptr<RegisterAllocationTracker> allocation_tracker_;
  const Editor* const editor_;
  const DominatorTree<Function>& dominator_tree_;

  // Allocatable registers ordered by less spilling heuristics.
  std::vector<Value> float_registers_;
  std::vector<Value> general_registers_;

  // Local allocation map
  LocalAllocation* local_map_;
  const LivenessCollection<BasicBlock*, Value>& liveness_;
  StackAllocator* const stack_allocator_;

  const RegisterUsageTracker& usage_tracker_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_
