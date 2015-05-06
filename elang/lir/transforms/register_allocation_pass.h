// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_PASS_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_PASS_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/work_list.h"
#include "elang/lir/pass.h"

namespace elang {
namespace lir {

class RegisterAssignments;
class StackAssignments;

//////////////////////////////////////////////////////////////////////
//
// RegisterAssignmentsPass
//
class ELANG_LIR_EXPORT RegisterAssignmentsPass final : public FunctionPass {
 public:
  explicit RegisterAssignmentsPass(base::StringPiece name, Editor* editor);
  ~RegisterAssignmentsPass() final;

 private:
  Value AssignmentOf(Instruction* instr, Value operand) const;

  // FunctionPass
  void RunOnFunction() final;

  void ProcessInstruction(Instruction* instr);

  std::unique_ptr<RegisterAssignments> register_assignments_;
  std::unique_ptr<StackAssignments> stack_assignments_;

  // List of useless instructions after register assignments.
  WorkList<Instruction> useless_instructions_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAssignmentsPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_PASS_H_
