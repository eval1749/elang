// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_H_

#include <algorithm>
#include <unordered_map>
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
typedef std::pair<Instruction*, Value> ValueLocation;

}  // namespace lir
}  // namespace elang

namespace std {
template <>
struct hash<elang::lir::ValueLocation> {
  size_t operator()(const elang::lir::ValueLocation& location) const;
};
}  // namespace std

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LocalAllocation contains virtual register allocation information
// at end of basic block.
//
class LocalAllocation final : public ZoneAllocated {
 public:
  ~LocalAllocation();

  Value PhysicalFor(Value vreg) const;
  Value StackSlotFor(Value vreg) const;

 private:
  friend class RegisterAllocationTracker;

  explicit LocalAllocation(Zone* zone);

  void RegisterAllocation(Value vreg, Value allocation);

  // Map virtual register to physical register.
  ZoneUnorderedMap<Value, Value> physical_map_;

  // Map virtual register to stack location.
  ZoneUnorderedMap<Value, Value> stack_slot_map_;

  DISALLOW_COPY_AND_ASSIGN(LocalAllocation);
};

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

  // Returns |LocalAllocation| of |block|.
  const LocalAllocation& AllocationsOf(BasicBlock* block) const;

  // Returns allocated value for |value| at |instr|.
  Value AllocationOf(Instruction* instr, Value value) const;

  // Returns |actions| executed before |instr|.
  const ZoneVector<Instruction*>& BeforeActionOf(Instruction* instr) const;

 private:
  friend class RegisterAllocationTracker;

  // Inserts |new_instr| before |ref_instr|.
  void InsertBefore(Instruction* new_instr, Instruction* ref_instr);

  void SetAllocation(Instruction* instr, Value vreg, Value allocated);

  std::unordered_map<Instruction*, Actions*> before_action_map_;
  std::unordered_map<BasicBlock*, LocalAllocation*> block_map_;
  std::unordered_map<ValueLocation, Value> map_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocation);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_H_
