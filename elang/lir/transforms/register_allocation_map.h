// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_MAP_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_MAP_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/base/zone_owner.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class RegisterAllocator struct Value;

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocationMap
//
class ELANG_LIR_EXPORT RegisterAllocationMap final : public ZoneOwner {
 public:
  struct Allocation {
    Value assignment;
    Value spill;
  };

  RegisterAllocationMap();
  ~RegisterAllocationMap();

  Allocation Get(Instruction* instruction, Value value);
  void Set(Instruction* instruction, Value value, Value physical);

 private:
  struct Location {
    Instruction* instruction;
    Value value;
  };

  std::unordered_map<Location, Allocation*> map_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocationMap);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_MAP_H_
