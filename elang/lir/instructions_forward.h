// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_LIR_INSTRUCTIONS_FORWARD_H_

#include <ostream>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

#define FOR_EACH_LIR_INSTRUCTION_0_0(V) \
  V(Entry, "entry")                     \
  V(Exit, "exit")

#define FOR_EACH_LIR_INSTRUCTION_0_1(V) V(Call, "call")

#define FOR_EACH_LIR_INSTRUCTION_0_2(V) V(Store, "store")

#define FOR_EACH_LIR_INSTRUCTION_1_1(V) \
  V(Copy, "mov")                        \
  V(Literal, "mov")                     \
  V(Load, "load")

#define FOR_EACH_LIR_INSTRUCTION_1_2(V) \
  V(Add, "add")                         \
  V(Div, "div")                         \
  V(Mod, "mod")                         \
  V(Mul, "mul")                         \
  V(Sub, "sub")                         \
  V(BitAnd, "and")                      \
  V(BitOr, "or")                        \
  V(BitXor, "xor")                      \
  V(Shl, "shl")                         \
  V(Shr, "shr")                         \
  V(Eq, "eq")                           \
  V(Ne, "ne")                           \
  V(Ge, "ge")                           \
  V(Gt, "gt")                           \
  V(Le, "le")                           \
  V(Lt, "lt")

#define FOR_EACH_LIR_INSTRUCTION_N_N(V)                                    \
  V(Branch, "br",                                                          \
    (Value condition, BasicBlock * true_block, BasicBlock * false_block))  \
  V(Jump, "jmp", (BasicBlock * target_block))                              \
  V(PCopy, "pcopy",                                                        \
    (const std::vector<Value>& outputs, const std::vector<Value>& inputs)) \
  V(Phi, "phi", (Value output))                                            \
  V(Ret, "ret", (BasicBlock* exit_block))

// Visitor |V| takes three parameters:
//  Name        capitalized name for C++ class
//  mnemonic    mnemonic of instruction
//  parameters  parameters for constructor
#define FOR_EACH_COMMON_LIR_INSTRUCTION(V) \
  FOR_EACH_LIR_INSTRUCTION_0_0(V)          \
  FOR_EACH_LIR_INSTRUCTION_0_1(V)          \
  FOR_EACH_LIR_INSTRUCTION_0_2(V)          \
  FOR_EACH_LIR_INSTRUCTION_1_1(V)          \
  FOR_EACH_LIR_INSTRUCTION_1_2(V)          \
  FOR_EACH_LIR_INSTRUCTION_N_N(V)

#ifdef ELANG_TARGET_ARCH_X64
////////////////////////////////////////////////////////////
//
// X64
//
#define FOR_EACH_X64_LIR_INSTRUCTION(V) \
  V(DivX64, "x64.div")                  \
  V(MulX64, "x64.mul")

#define FOR_EACH_LIR_INSTRUCTION(V)  \
  FOR_EACH_COMMON_LIR_INSTRUCTION(V) \
  FOR_EACH_X64_LIR_INSTRUCTION(V)

#else
#error "You should define known ELANG_TARGET_ARCH_XXX."
#endif

// Forward declarations
// Forward declarations
class Instruction;
class InstructionVisitor;
enum class Opcode;
class PhiInstructionList;
#define V(Name, ...) class Name##Instruction;
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream, Opcode opcode);

ELANG_LIR_EXPORT base::StringPiece ToStringPiece(Opcode opcode);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_FORWARD_H_
