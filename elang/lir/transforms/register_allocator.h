// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_

#include <memory>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/lir_export.h"

namespace elang {

template <typename Graph>
class DominatorTree;

template <typename BasicBlock, typename Value>
class LivenessCollection;

namespace lir {

class BasicBlock;
class ConflictMap;
class Editor;
class Factory;
class Function;
class Instruction;
class LocalAllocation;
class RegisterAssignments;
class RegisterAllocationTracker;
class RegisterUsageTracker;
class SpillManager;
class StackAllocator;
class StackAssignments;
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
//  and store them in |RegisterAssignments| instance via
//  |RegisterAllocationTracker|.
//
// Prerequisite:
//  - Predecessors of phi block must end with unconditional branch.
//  - No users virtual register must not be appeared.
//
class ELANG_LIR_EXPORT RegisterAllocator final : public InstructionVisitor {
 public:
  RegisterAllocator(Editor* editor,
                    RegisterAssignments* register_assignments,
                    StackAssignments* stack_assignments);
  ~RegisterAllocator() final;

  // The entry point
  void Run();

 private:
  typedef std::pair<Value, Value> ValuePair;

  Factory* factory() const;
  Function* function() const;

  // Returns list of all allocatable natural registers for |type|. Returned list
  // contains both allocated and free registers.
  const std::vector<Value>& AllocatableRegistersFor(Value type) const;
  // Returns physical register of if 'assign' instruction just before |instr|,
  // otherwise returns void.
  Value AssignedPhysicalFor(Instruction* instr);
  // Returns callee save register for |vreg| if available otherwise returns
  // void.
  Value CalleeSavedRegisterFor(Value vreg) const;
  Value EnsureSpillSlot(Value vreg);
  void ExpandParallelCopy(const std::vector<ValuePair>& pairs,
                          Instruction* ref_instr);
  void FreeInputOperandsIfNotUsed(Instruction* instruction);
  bool HasBackEdge(BasicBlock* block) const;
  bool IsBackEdge(BasicBlock* from, BasicBlock* to) const;
  void MustAllocate(Instruction* instruction, Value output, Value physical);
  void PopulateAllocationMap(BasicBlock* block);
  void ProcessBlock(BasicBlock* block);
  void ProcessInputOperand(Instruction* instruction, Value input, int position);
  void ProcessInputOperands(Instruction* instruction);
  void ProcessOutputOperand(Instruction* instruction, Value output);
  void ProcessOutputOperands(Instruction* instruction);
  void ProcessPhiInputOperands(BasicBlock* block, BasicBlock* predecessor);
  void ProcessPhiOutputOperands(BasicBlock* block);
  void ProcessPredecessors(BasicBlock* block);
  void SortAllocatableRegisters();

  // Returns true if |output| is allocated to |physical|, or returns false.
  bool TryAllocate(Instruction* instruction, Value output, Value physical);

  ////////////////////////////////////////////////////////////
  //
  // Spill and Reload
  //
  Value ChooseRegisterToSpill(Instruction* instruction, Value type) const;

  // Spill physical register allocated to virtual register |victim| before
  // |instruction|.
  Value Spill(Instruction* instruction, Value victim);
  Instruction* NewSpill(Instruction* instr, Value spill_slot, Value physical);

  ////////////////////////////////////////////////////////////
  //
  // Query allocation for virtual register.
  //

  // Returns physical register or stacl slot for virtual register, or |value|
  // if |value| isn't a virtual register.
  Value AllocationOf(Value value) const;

  // Returns allocated physical register for |vreg|, or void if not allocated.
  Value PhysicalFor(Value vreg) const;

  // Returns allocated stack slot for |vreg|, or void if not allocated.
  Value SpillSlotFor(Value vreg) const;

  Value VirtualFor(Value physical) const;

  // InstructionVisitor
  void DoDefaultVisit(Instruction* instruction);
  void VisitCall(CallInstruction* instr);
  void VisitCopy(CopyInstruction* instr);
  void VisitPCopy(PCopyInstruction* instr);
  void VisitUse(UseInstruction* instr);

  std::unique_ptr<RegisterAllocationTracker> allocation_tracker_;
  const DominatorTree<Function>& dominator_tree_;
  const Editor* const editor_;

  // Allocatable registers ordered by less spilling heuristics.
  std::vector<Value> float_registers_;
  std::vector<Value> general_registers_;

  // Local allocation map
  LocalAllocation* local_map_;
  const LivenessCollection<BasicBlock*, Value>& liveness_;
  const std::unique_ptr<StackAllocator> stack_allocator_;
  const std::unique_ptr<SpillManager> spill_manager_;
  const std::unique_ptr<RegisterUsageTracker> usage_tracker_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_
