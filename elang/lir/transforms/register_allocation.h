// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_H_

#include <algorithm>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class BasicBlock;
class Instruction;
class RegisterAllocationTracker;
typedef std::pair<BasicBlock*, Value> BasicBlockValue;
typedef std::pair<Instruction*, Value> InstructionValue;
}  // namespace lir
}  // namespace elang

namespace std {
template <>
struct hash<elang::lir::BasicBlockValue> {
  size_t operator()(const elang::lir::BasicBlockValue& location) const;
};

template <>
struct hash<elang::lir::InstructionValue> {
  size_t operator()(const elang::lir::InstructionValue& location) const;
};
}  // namespace std

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocation
//
class ELANG_LIR_EXPORT RegisterAllocation final : public ZoneOwner {
 public:
  struct Actions : ZoneAllocated {
    ZoneVector<Instruction*> actions;

    explicit Actions(Zone* zone);
  };

  RegisterAllocation();
  ~RegisterAllocation();

  // Returns allocated value for |value| after last instruction.
  Value AllocationOf(BasicBlock* block, Value value) const;

  // Returns allocated value for |value| at |instr|.
  Value AllocationOf(Instruction* instr, Value value) const;

  // Returns |actions| executed before |instr|.
  const ZoneVector<Instruction*>& BeforeActionOf(Instruction* instr) const;

 private:
  friend class RegisterAllocationTracker;

  // Inserts |new_instr| before |ref_instr|.
  void InsertBefore(Instruction* new_instr, Instruction* ref_instr);

  void SetAllocation(Instruction* instr, Value vreg, Value allocation);
  void SetPhysical(BasicBlock* block, Value vreg, Value physical);
  void SetStackSlot(Value vreg, Value stack_slot);

  ZoneUnorderedMap<BasicBlockValue, Value> block_value_map_;
  ZoneUnorderedMap<Instruction*, Actions*> before_action_map_;
  ZoneVector<Instruction*> empty_actions_;
  ZoneUnorderedMap<InstructionValue, Value> instruction_value_map_;

  // Map virtual register to stack location.
  ZoneUnorderedMap<Value, Value> stack_slot_map_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocation);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_H_
