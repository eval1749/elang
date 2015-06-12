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

#define FOR_EACH_LIR_INSTRUCTION_0_0(V) V(Exit, "exit")
#define FOR_EACH_LIR_INSTRUCTION_0_1(V) V(Use, "use")
#define FOR_EACH_LIR_INSTRUCTION_0_4(V) V(Store, "store")

#define FOR_EACH_LIR_INSTRUCTION_1_1(V) \
  V(Assign, "assign")                   \
  V(Copy, "mov")                        \
  V(Extend, "ext")                      \
  V(Literal, "lit")                     \
  V(SignedConvert, "sconv")             \
  V(SignExtend, "sext")                 \
  V(Truncate, "trunc")                  \
  V(UnsignedConvert, "uconv")           \
  V(ZeroExtend, "zext")

#define FOR_EACH_LIR_INSTRUCTION_1_2(V) \
  V(Add, "add")                         \
  V(BitAnd, "and")                      \
  V(BitOr, "or")                        \
  V(BitXor, "xor")                      \
  V(FloatAdd, "fadd")                   \
  V(FloatDiv, "fdiv")                   \
  V(FloatMod, "fmod")                   \
  V(FloatMul, "fmul")                   \
  V(FloatSub, "fsub")                   \
  V(Div, "div")                         \
  V(Mod, "mod")                         \
  V(Mul, "mul")                         \
  V(Sub, "sub")                         \
  V(Shl, "shl")                         \
  V(Shr, "shr")                         \
  V(UIntDiv, "udiv")                    \
  V(UIntMod, "umod")                    \
  V(UIntMul, "umul")                    \
  V(UIntShr, "ushr")

#define FOR_EACH_LIR_INSTRUCTION_1_3(V) V(Load, "load")

#define FOR_EACH_LIR_INSTRUCTION_N_N(V)                                    \
  V(Branch, "br",                                                          \
    (Value condition, BasicBlock * true_block, BasicBlock * false_block))  \
  V(Call, "call", (const std::vector<Value>& output, Value callee))        \
  V(Cmp, "cmp",                                                            \
    (Value output, IntCondition condition, Value left, Value right))       \
  V(Entry, "entry", (const std::vector<Value>& outputs))                   \
  V(FloatCmp, "FloatCmp",                                                  \
    (Value output, FloatCondition condition, Value left, Value right))     \
  V(Jump, "jmp", (BasicBlock * target_block))                              \
  V(PCopy, "pcopy",                                                        \
    (const std::vector<Value>& outputs, const std::vector<Value>& inputs)) \
  V(Phi, "phi", (Value output))                                            \
  V(Ret, "ret", (BasicBlock * exit_block))

// Visitor |V| takes three parameters:
//  Name        capitalized name for C++ class
//  mnemonic    mnemonic of instruction
//  parameters  parameters for constructor
#define FOR_EACH_COMMON_LIR_INSTRUCTION(V) \
  FOR_EACH_LIR_INSTRUCTION_0_0(V)          \
  FOR_EACH_LIR_INSTRUCTION_0_1(V)          \
  FOR_EACH_LIR_INSTRUCTION_0_4(V)          \
  FOR_EACH_LIR_INSTRUCTION_1_1(V)          \
  FOR_EACH_LIR_INSTRUCTION_1_2(V)          \
  FOR_EACH_LIR_INSTRUCTION_1_3(V)          \
  FOR_EACH_LIR_INSTRUCTION_N_N(V)

#ifdef ELANG_TARGET_ARCH_X64
////////////////////////////////////////////////////////////
//
// X64
//
#define FOR_EACH_X64_LIR_INSTRUCTION(V) V(DivX64, "x64.div")

#define FOR_EACH_LIR_INSTRUCTION(V)  \
  FOR_EACH_COMMON_LIR_INSTRUCTION(V) \
  FOR_EACH_X64_LIR_INSTRUCTION(V)

#else
#error "You should define known ELANG_TARGET_ARCH_XXX."
#endif

// Forward declarations
enum class FloatCondition;
enum class IntCondition;
class Instruction;
class InstructionVisitor;
enum class Opcode;
class PhiInstructionList;
#define V(Name, ...) class Name##Instruction;
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction* instruction);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          IntCondition condition);
ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream, Opcode opcode);

ELANG_LIR_EXPORT base::StringPiece ToStringPiece(Opcode opcode);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_FORWARD_H_
