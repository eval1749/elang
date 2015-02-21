// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_STACK_ASSIGNMENTS_H_
#define ELANG_LIR_TRANSFORMS_STACK_ASSIGNMENTS_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/macros.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class Instruction;

//////////////////////////////////////////////////////////////////////
//
// StackAssignments
//
class ELANG_LIR_EXPORT StackAssignments final {
 public:
  StackAssignments();
  ~StackAssignments();

  const std::unordered_set<Value>& arguments() const { return arguments_; }
  const std::vector<Instruction*> epilogue() const {
    return epilogue_instructions_;
  }
  int maximum_arguments_size() const { return maximum_arguments_size_; }
  int maximum_variables_size() const { return maximum_variables_size_; }
  int number_of_calls() const { return number_of_calls_; }
  int number_of_parameters() const { return number_of_parameters_; }
  const std::unordered_map<Value, Value>& preserving_registers() const {
    return preserving_registers_;
  }
  const std::vector<Instruction*> prologue() const {
    return prologue_instructions_;
  }

  Value StackSlotOf(Value proxy) const;

 private:
  friend class StackAllocator;
  friend class StackAssigner;

  std::unordered_set<Value> arguments_;
  std::vector<Instruction*> epilogue_instructions_;
  int maximum_arguments_size_;
  int maximum_variables_size_;
  int number_of_calls_;
  // Number of stack slots used for parameter passing.
  int number_of_parameters_;
  std::vector<Instruction*> prologue_instructions_;

  // Mapping from physical register to proxy slot.
  std::unordered_map<Value, Value> preserving_registers_;

  // Mapping from memory proxy to stack slot.
  std::unordered_map<Value, Value> stack_map_;

  DISALLOW_COPY_AND_ASSIGN(StackAssignments);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_STACK_ASSIGNMENTS_H_
