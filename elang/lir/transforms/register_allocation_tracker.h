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
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class BasicBlock;
class Instruction;
class RegisterAssignments;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocationTracker
//
class ELANG_LIR_EXPORT RegisterAllocationTracker final {
 public:
  explicit RegisterAllocationTracker(RegisterAssignments* allocation_map);
  ~RegisterAllocationTracker();

  // Returns map from virtual register to physical register.
  const std::unordered_map<Value, Value>& physical_map() const;

  Value AllocationOf(BasicBlock* block, Value vreg) const;
  Value AllocationOf(Instruction* instr, Value vreg) const;
  void EndBlock(BasicBlock* block);
  void StartBlock(BasicBlock* block);
  void SetPhysical(BasicBlock* block, Value vreg, Value physical);
  void SetSpillSlot(Value virtual_register, Value spill_slot);

  // Query current mapping
  Value AllocationOf(Value virtual_register) const;
  Value PhysicalFor(Value virtual_register) const;
  Value SpillSlotFor(Value virtual_register) const;

  // Expose for |RegisterAllocate::TryAllocate()|.
  Value VirtualFor(Value physical) const;

  // Update current mapping
  void FreeVirtual(Value virtual_register);
  void FreePhysical(Value physical);
  void TrackPhysical(Value virtual_register, Value physical);

  // Recording allocation
  void InsertBefore(Instruction* new_instr, Instruction* ref_instr);
  void SetAllocation(Instruction* instr, Value vreg, Value allocated);

 private:
  // Result of register allocation.
  RegisterAssignments::Editor assignments_;

  // Map virtual register to physical register.
  std::unordered_map<Value, Value> physical_map_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocationTracker);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_TRACKER_H_
