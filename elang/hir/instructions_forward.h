// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_HIR_INSTRUCTIONS_FORWARD_H_

#include <ostream>
#include <string>

#include "elang/base/float_types.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

// HIR doesn't have unary operators. Unary operators are represented by binary
// operator as follows:
//  -x  sub %out = 0, x
//  ~x  xor %out = x, -1
//  !x  eq %out = x, false
//
// Note: Compilers may generate compilation error negation of unsigned integer.

// To generate instruction constructor, validator, class, we have macros
// categorized by output and input operands types.

// Visitor |V| takes three parameters:
//  Name         capitalized name for C++ class
//  mnemonic     mnemonic; used for formatting
// Note: inputs of |PhiInstruction| are stored in |PhiInput|.

// Types of output and inputs are must be same numerical type.
#define FOR_EACH_ARITHMETIC_BINARY_OPERATION(V) \
  V(Add, "add")                                 \
  V(Div, "div")                                 \
  V(Mod, "mod")                                 \
  V(Mul, "mul")                                 \
  V(Sub, "sub")

// Types of output and inputs are must be same integer type.
#define FOR_EACH_BITWISE_BINARY_OPERATION(V) \
  V(BitAnd, "and")                           \
  V(BitOr, "ior")                            \
  V(BitXor, "xor")

// Types of output and input(0) are must be same integer type and input(1)
// must be int32.
#define FOR_EACH_BITWISE_SHIFT_OPERATION(V) \
  V(Shl, "shl")                             \
  V(Shr, "shr")

// Output type is bool and input(0) and input(1) must be same type.
#define FOR_EACH_EQUALITY_OPERATION(V) \
  V(Eq, "eq")                          \
  V(Ne, "ne")

// Output type is bool and input(0) and input(1) must be same numerical type.
#define FOR_EACH_RELATIONAL_OPERATION(V) \
  V(Ge, "ge")                            \
  V(Gt, "gt")                            \
  V(Le, "le")                            \
  V(Lt, "lt")

#define FOR_EACH_TYPE_CAST_OPERATION(V) \
  V(DynamicCast, "dynamic_cast")        \
  V(StaticCast, "static_cast")

// Instructions which have simple C++ constructor.
#define FOR_EACH_SIMPLE_HIR_INSTRUCTION(V) \
  FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)  \
  FOR_EACH_BITWISE_BINARY_OPERATION(V)     \
  FOR_EACH_BITWISE_SHIFT_OPERATION(V)      \
  FOR_EACH_EQUALITY_OPERATION(V)           \
  FOR_EACH_RELATIONAL_OPERATION(V)         \
  FOR_EACH_TYPE_CAST_OPERATION(V)          \
  V(Branch, "br")                          \
  V(Call, "call")                          \
  V(Entry, "entry")                        \
  V(Exit, "exit")                          \
  V(Jump, "br")                            \
  V(If, "if")                              \
  V(Load, "load")                          \
  V(Return, "ret")                         \
  V(Store, "store")                        \
  V(Unreachable, "unreachable")

#define FOR_EACH_HIR_INSTRUCTION(V)  \
  FOR_EACH_SIMPLE_HIR_INSTRUCTION(V) \
  V(Get, "get")                      \
  V(Phi, "phi")                      \
  V(StackAlloc, "alloca")            \
  V(Tuple, "tuple")

#define V(Name, ...) class Name##Instruction;
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

// Forward declarations
class Instruction;
class InstructionVisitor;
enum class Opcode;
class OperandIterator;
class Operands;
class PhiInput;

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream, Opcode opcode);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_FORWARD_H_
