// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_LIR_INSTRUCTIONS_FORWARD_H_

#include <ostream>

#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

// Forward declarations
class Instruction;
class InstructionVisitor;

#define FOR_EACH_LIR_INSTRUCTION_0_0(V) \
  V(Entry)                              \
  V(Exit)                               \
  V(Ret)

#define FOR_EACH_LIR_INSTRUCTION_0_1(V) V(Call)

#define FOR_EACH_LIR_INSTRUCTION_1_1(V) \
  V(Copy)                               \
  V(Load)

#define FOR_EACH_LIR_INSTRUCTION_N(V) V(Jump, (BasicBlock * target_block))

// Visitor |V| takes three parameters:
//  Name        capitalized name for C++ class
//  parameters  parameters for constructor
#define FOR_EACH_LIR_INSTRUCTION(V) \
  FOR_EACH_LIR_INSTRUCTION_0_0(V)   \
  FOR_EACH_LIR_INSTRUCTION_0_1(V)   \
  FOR_EACH_LIR_INSTRUCTION_1_1(V)   \
  FOR_EACH_LIR_INSTRUCTION_N(V)

#define V(Name, ...) class Name##Instruction;
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_FORWARD_H_
