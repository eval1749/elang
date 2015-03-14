// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_PHI_EXPANDER_H_
#define ELANG_LIR_TRANSFORMS_PHI_EXPANDER_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/macros.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class BasicBlock;
class Instruction;
class RegisterAllocationTracker;
class SpillManager;

//////////////////////////////////////////////////////////////////////
//
// PhiExpander
//
class PhiExpander final {
 public:
  PhiExpander(RegisterAllocationTracker* allocation_tracker,
              SpillManager* spill_manager,
              BasicBlock* phi_block,
              BasicBlock* predecessor);
  ~PhiExpander();

  void AddRegister(Value physical);
  void Expand();

 private:
  Value AllocationOf(Value value) const;

  void EmitReload(Value physical, Value vreg);
  void EmitSpill(Value vreg, Value physical);

  // Choose spill register
  Value ChooseSpillRegisterFromInput(Value type) const;
  Value ChooseSpillRegisterFromLiveIn(Value type) const;
  Value ChooseSpillRegisterFromOutput(Value type) const;

  // Returns true if |physical| is used as input operand.
  bool IsInput(Value physical) const;

  // Spill
  bool SpillFromInput(Value type);
  void SpillFromLiveIn(Value type);
  bool SpillFromOutput(Value type);

  // Returns physical register allocated to |vreg|.
  Value UpdateAllocationForSpill(Value vreg, Value spill_slot);

  std::unordered_map<Value, Value> allocations_;
  RegisterAllocationTracker* const allocation_tracker_;
  std::unordered_set<Value> input_registers_;
  std::unordered_set<Value> live_registers_;
  std::unordered_set<Value> output_registers_;
  BasicBlock* const phi_block_;
  std::unordered_set<Value> phi_registers_;
  BasicBlock* const predecessor_;
  std::vector<Instruction*> reloads_;
  std::unordered_set<Value> scratch_registers_;
  std::vector<Instruction*> spills_;
  SpillManager* const spill_manager_;

  DISALLOW_COPY_AND_ASSIGN(PhiExpander);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_PHI_EXPANDER_H_
