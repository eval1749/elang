// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_STACK_ALLOCATOR_H_
#define ELANG_LIR_TRANSFORMS_STACK_ALLOCATOR_H_

#include <algorithm>
#include <set>
#include <unordered_map>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class Editor;
class ConflictMap;
class Instruction;
class StackAssignments;

//////////////////////////////////////////////////////////////////////
//
// StackAllocator
//
class ELANG_LIR_EXPORT StackAllocator final : public ZoneOwner {
 public:
  StackAllocator(const Editor* editor, StackAssignments* assignments);
  ~StackAllocator();

  Value AllocationFor(Value vreg) const;
  Value Allocate(Value vreg);
  void AllocateForPreserving(Value physical);
  void Assign(Value vreg, Value proxy);
  void Free(Value vreg);
  void Reallocate(Value vreg, Value proxy);
  void Reset();
  void TrackCall(Instruction* instruction);

 private:
  struct Slot : ZoneAllocated {
    Value proxy;
    ZoneVector<Value> users;

    explicit Slot(Zone* zone) : users(zone) {}
  };

  struct SlotLess {
    bool operator()(const Slot* a, const Slot* b) const {
      return a->proxy.data < b->proxy.data;
    }
  };

  // Returns |Slot| for |vreg| or null if unavailable.
  Slot* FreeSlotFor(Value vreg) const;

  // Returns true if users of |slot| are conflicted to |vreg|.
  bool IsConflict(const Slot* slot, Value vreg) const;

  // Returns newly allocated spill slot which can hold value of |type|.
  Slot* NewSlot(Value type);
  void TrackArgument(Value proxy);

  int const alignment_;
  StackAssignments* const assignments_;
  const ConflictMap& conflict_map_;
  std::set<Slot*, SlotLess> free_slots_;
  std::set<Slot*, SlotLess> live_slots_;
  int size_;

  // Map virtual register to memory proxy.
  std::unordered_map<Value, Slot*> slot_map_;

  DISALLOW_COPY_AND_ASSIGN(StackAllocator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_STACK_ALLOCATOR_H_
