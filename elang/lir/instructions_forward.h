// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_LIR_INSTRUCTIONS_FORWARD_H_

#include <ostream>
#include <vector>

#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

// Forward declarations
class Instruction;
class InstructionVisitor;
class PhiInstructionList;

#define FOR_EACH_LIR_INSTRUCTION_0_0(V) \
  V(Entry)                              \
  V(Exit)                               \
  V(Ret)

#define FOR_EACH_LIR_INSTRUCTION_0_1(V) V(Call)
#define FOR_EACH_LIR_INSTRUCTION_0_2(V) V(Store)

#define FOR_EACH_LIR_INSTRUCTION_1_1(V) \
  V(Copy)                               \
  V(Literal)                            \
  V(Load)

#define FOR_EACH_LIR_INSTRUCTION_1_2(V) \
  V(Add)                                \
  V(Div)                                \
  V(Mod)                                \
  V(Mul)                                \
  V(Sub)                                \
  V(BitAnd)                             \
  V(BitOr)                              \
  V(BitXor)                             \
  V(Shl)                                \
  V(Shr)                                \
  V(Eq)                                 \
  V(Ne)                                 \
  V(Ge)                                 \
  V(Gt)                                 \
  V(Le)                                 \
  V(Lt)

#define FOR_EACH_LIR_INSTRUCTION_N_N(V)                                    \
  V(Branch,                                                                \
    (Value condition, BasicBlock * true_block, BasicBlock * false_block))  \
  V(Jump, (BasicBlock * target_block))                                     \
  V(PCopy,                                                                 \
    (const std::vector<Value>& outputs, const std::vector<Value>& inputs)) \
  V(Phi, (Value output))

// Visitor |V| takes three parameters:
//  Name        capitalized name for C++ class
//  parameters  parameters for constructor
#define FOR_EACH_COMMON_LIR_INSTRUCTION(V) \
  FOR_EACH_LIR_INSTRUCTION_0_0(V)          \
  FOR_EACH_LIR_INSTRUCTION_0_1(V)          \
  FOR_EACH_LIR_INSTRUCTION_0_2(V)          \
  FOR_EACH_LIR_INSTRUCTION_1_1(V)          \
  FOR_EACH_LIR_INSTRUCTION_1_2(V)          \
  FOR_EACH_LIR_INSTRUCTION_N_N(V)

#define V(Name, ...) class Name##Instruction;
FOR_EACH_COMMON_LIR_INSTRUCTION(V)
#undef V

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_FORWARD_H_
