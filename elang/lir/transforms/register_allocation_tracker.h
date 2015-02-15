// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_TRACKER_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_TRACKER_H_

#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class BasicBlock;
class Instruction;
class RegisterAllocation;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocationTracker
//
class ELANG_LIR_EXPORT RegisterAllocationTracker final {
 public:
  explicit RegisterAllocationTracker(RegisterAllocation* allocation_map);
  ~RegisterAllocationTracker();

  // Returns map from virtual register to physical register.
  const std::unordered_map<Value, Value>& physical_map() const;

  Value AllocationOf(BasicBlock* block, Value vreg) const;
  void EndBlock(BasicBlock* block);
  void StartBlock(BasicBlock* block);
  void SetPhysical(BasicBlock* block, Value vreg, Value physical);

  // Query current mapping
  Value AllocationOf(Value virtual_register) const;
  Value PhysicalFor(Value virtual_register) const;
  Value StackSlotFor(Value virtual_register) const;

  // Expose for testing purpose.
  Value VirtualFor(Value physical) const;

  // Update current mapping
  void FreeVirtual(Value virtual_register);
  void FreePhysical(Value physical);
  void TrackPhysical(Value virtual_register, Value physical);
  void TrackStackSlot(Value virtual_register, Value stack_slot);

  // Returns true if |output| is allocated to |physical|, otherwise returns
  // false.
  bool TryAllocate(Instruction* instr, Value output, Value physical);

  // Recording allocation
  void InsertBefore(Instruction* new_instr, Instruction* ref_instr);
  void SetAllocation(Instruction* instr, Value vreg, Value allocated);

 private:
  // Result of register allocation.
  RegisterAllocation* const allocations_;

  // Map virtual register to physical register.
  std::unordered_map<Value, Value> physical_map_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocationTracker);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_TRACKER_H_
