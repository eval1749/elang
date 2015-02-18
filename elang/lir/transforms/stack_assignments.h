// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_STACK_ASSIGNMENTS_H_
#define ELANG_LIR_TRANSFORMS_STACK_ASSIGNMENTS_H_

#include <vector>

#include "base/macros.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class Editor;
class Instruction;

//////////////////////////////////////////////////////////////////////
//
// StackAssignments
//
class ELANG_LIR_EXPORT StackAssignments final {
 public:
  explicit StackAssignments(const Editor* editor);
  ~StackAssignments();

  int maximum_argc() const { return maximum_argc_; }
  int maximum_size() const { return maximum_size_; }
  const std::vector<Value>& preserving_registers() const {
    return preserving_registers_;
  }

 private:
  friend class StackAllocator;
  friend class StackAssigner;

  std::vector<Instruction*> epilogue_instructions_;
  const Editor* const editor_;
  int maximum_argc_;
  int maximum_size_;
  std::vector<Instruction*> prologue_instructions_;
  std::vector<Value> preserving_registers_;
  int slot_count_;

  DISALLOW_COPY_AND_ASSIGN(StackAssignments);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_STACK_ASSIGNMENTS_H_
