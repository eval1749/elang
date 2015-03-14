// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_SPILL_MANAGER_H_
#define ELANG_LIR_TRANSFORMS_SPILL_MANAGER_H_

#include "base/macros.h"

namespace elang {
namespace lir {

class Factory;
class Instruction;
class RegisterAllocationTracker;
class RegisterUsageTracker;
class StackAllocator;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// SpillManager
//
class SpillManager final {
 public:
  explicit SpillManager(Factory* factory,
                        RegisterAllocationTracker* allocation_tracker,
                        StackAllocator* stack_allocator,
                        RegisterUsageTracker* usage_tracker);
  ~SpillManager();

  Factory* factory() const { return factory_; }

  Value ChooseRegisterToSpill(Value type, Instruction* instr) const;

  // Returns spill slot |vreg|. If |vreg| doesn't have spill slot, this function
  // allocates spill slot |vreg|.
  Value EnsureSpillSlot(Value vreg);

  // Returns a newly created instruction to load |physical| from spill
  // slot for |vreg|.
  Instruction* NewReload(Value physical, Value vreg);

  // Returns a newly created instruction to store |physical| to spill
  // slot for |vreg|.
  Instruction* NewSpill(Value spill_slot, Value physical);

  Value SpillSlotFor(Value vreg) const;

 private:
  RegisterAllocationTracker* const allocation_tracker_;
  Factory* const factory_;
  StackAllocator* const stack_allocator_;
  RegisterUsageTracker* const usage_tracker_;

  DISALLOW_COPY_AND_ASSIGN(SpillManager);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_SPILL_MANAGER_H_
