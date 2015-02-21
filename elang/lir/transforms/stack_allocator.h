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
  void Assign(Value vreg, Value spill_slot);
  void Free(Value vreg);
  void Reset();
  void TrackNumberOfArguments(int number_of_arguments);

 private:
  struct Slot : ZoneAllocated {
    Value spill_slot;
    ZoneVector<Value> users;

    explicit Slot(Zone* zone) : users(zone) {}
  };

  struct SlotLess {
    bool operator()(const Slot* a, const Slot* b) const {
      return a->spill_slot.data < b->spill_slot.data;
    }
  };

  bool IsConflict(const Slot* slot, Value vreg) const;

  int const alignment_;
  StackAssignments* const assignments_;
  const ConflictMap& conflict_map_;
  std::set<Slot*, SlotLess> free_slots_;
  std::set<Slot*, SlotLess> live_slots_;
  int size_;
  std::unordered_map<Value, Slot*> slot_map_;

  DISALLOW_COPY_AND_ASSIGN(StackAllocator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_STACK_ALLOCATOR_H_
