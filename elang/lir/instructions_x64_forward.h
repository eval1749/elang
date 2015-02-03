// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_X64_FORWARD_H_
#define ELANG_LIR_INSTRUCTIONS_X64_FORWARD_H_

#include "elang/lir/instructions_forward.h"

namespace elang {
namespace lir {

#define FOR_EACH_X64_LIR_INSTRUCTION(V) \
  V(Div2)                               \
  V(Mul2)

#define V(Name) class Name##Instruction;
FOR_EACH_X64_LIR_INSTRUCTION(V)
#undef V

#define FOR_EACH_LIR_INSTRUCTION(V)  \
  FOR_EACH_COMMON_LIR_INSTRUCTION(V) \
  FOR_EACH_X64_LIR_INSTRUCTION(V)

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_X64_FORWARD_H_
