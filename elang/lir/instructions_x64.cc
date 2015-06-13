// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/instructions_x64.h"
#include "elang/lir/target_x64.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

// IntDivX64
IntDivX64Instruction::IntDivX64Instruction(Value div_output,
                                           Value mod_output,
                                           Value high_left,
                                           Value low_left,
                                           Value right) {
  InitOutput(0, div_output);
  InitOutput(1, mod_output);
  InitInput(0, high_left);
  InitInput(1, low_left);
  InitInput(2, right);
}

// IntSignX64
IntSignX64Instruction::IntSignX64Instruction(Value output, Value input) {
  InitOutput(0, output);
  InitInput(0, input);
}

// UIntDivX64
UIntDivX64Instruction::UIntDivX64Instruction(Value div_output,
                                             Value mod_output,
                                             Value high_left,
                                             Value low_left,
                                             Value right) {
  InitOutput(0, div_output);
  InitOutput(1, mod_output);
  InitInput(0, high_left);
  InitInput(1, low_left);
  InitInput(2, right);
}

// UIntMulX64
UIntMulX64Instruction::UIntMulX64Instruction(Value high_output,
                                             Value low_output,
                                             Value left,
                                             Value right) {
  InitOutput(0, high_output);
  InitOutput(1, low_output);
  InitInput(0, left);
  InitInput(1, right);
}

}  // namespace lir
}  // namespace elang
