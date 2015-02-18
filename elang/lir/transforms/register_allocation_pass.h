// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_PASS_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_PASS_H_

#include <vector>

#include "base/macros.h"
#include "elang/lir/pass.h"

namespace elang {
namespace lir {

class RegisterAssignments;

//////////////////////////////////////////////////////////////////////
//
// RegisterAssignmentsPass
//
class ELANG_LIR_EXPORT RegisterAssignmentsPass final : public FunctionPass {
 public:
  explicit RegisterAssignmentsPass(Editor* editor);
  ~RegisterAssignmentsPass() final;

 private:
  // Pass
  base::StringPiece name() const final;

  // Function
  void RunOnFunction() final;

  void ProcessInstruction(const RegisterAssignments& allocations,
                          Instruction* instr);

  DISALLOW_COPY_AND_ASSIGN(RegisterAssignmentsPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATION_PASS_H_
