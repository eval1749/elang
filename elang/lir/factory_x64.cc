// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/factory.h"

#include "elang/lir/instructions_x64.h"

#ifndef ELANG_TARGET_ARCH_X64
#error "You should define ELANG_TARGET_ARCH_X64"
#endif

namespace elang {
namespace lir {

Instruction* Factory::NewIntDivX64Instruction(Value div_output,
                                              Value mod_output,
                                              Value high_left,
                                              Value low_left,
                                              Value right) {
  return new (zone())
      IntDivX64Instruction(div_output, mod_output, high_left, low_left, right);
}

Instruction* Factory::NewSignX64Instruction(Value output, Value input) {
  return new (zone()) SignX64Instruction(output, input);
}

Instruction* Factory::NewUIntDivX64Instruction(Value div_output,
                                               Value mod_output,
                                               Value high_left,
                                               Value low_left,
                                               Value right) {
  return new (zone())
      UIntDivX64Instruction(div_output, mod_output, high_left, low_left, right);
}

Instruction* Factory::NewUIntMulX64Instruction(Value high_output,
                                               Value low_output,
                                               Value left,
                                               Value right) {
  return new (zone())
      UIntMulX64Instruction(high_output, low_output, left, right);
}

}  // namespace lir
}  // namespace elang
