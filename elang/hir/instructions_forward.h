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

// Types of output and inputs are must be same numerical type.
#define FOR_EACH_ARITHMETIC_BINARY_OPERATION(V) \
  V(Add, "add", Fixed)                          \
  V(Div, "div", Fixed)                          \
  V(Mod, "mod", Fixed)                          \
  V(Mul, "mul", Fixed)                          \
  V(Sub, "sub", Fixed)

// Types of output and inputs are must be same integer type.
#define FOR_EACH_BITWISE_BINARY_OPERATION(V) \
  V(BitAnd, "and", Fixed)                    \
  V(BitOr, "ior", Fixed)                     \
  V(BitXor, "xor", Fixed)

// Types of output and input(0) are must be same integer type and input(1)
// must be int32.
#define FOR_EACH_BITWISE_SHIFT_OPERATION(V) \
  V(Shl, "shl", Fixed)                      \
  V(Shr, "shr", Fixed)

// Output type is bool and input(0) and input(1) must be same type.
#define FOR_EACH_EQUALITY_OPERATION(V) \
  V(Eq, "eq", Fixed)                   \
  V(Ne, "ne", Fixed)

// Output type is bool and input(0) and input(1) must be same numerical type.
#define FOR_EACH_RELATIONAL_OPERATION(V) \
  V(Ge, "ge", Fixed)                     \
  V(Gt, "gt", Fixed)                     \
  V(Le, "le", Fixed)                     \
  V(Lt, "lt", Fixed)

#define FOR_EACH_TYPE_CAST_OPERATION(V) \
  V(DynamicCast, "dynamic_cast", Fixed) \
  V(StaticCast, "static_cast", Fixed)

// Visitor |V| takes three parameters:
//  Name         capitalized name for C++ class
//  mnemonic     mnemonic; used for formatting
// Note: inputs of |PhiInstruction| are stored in |PhiInput|.
#define FOR_EACH_HIR_INSTRUCTION(V)       \
  V(Branch, "br", Fixed)                  \
  V(Call, "call", Fixed)                  \
  V(Entry, "entry", Fixed)                \
  V(Exit, "exit", Fixed)                  \
  V(Get, "get", Fixed)                    \
  V(Jump, "br", Fixed)                    \
  V(If, "if", Fixed)                      \
  V(Load, "load", Fixed)                  \
  V(Phi, "phi", Fixed)                    \
  V(Return, "ret", Fixed)                 \
  V(StackAlloc, "alloca", Fixed)          \
  V(Store, "store", Fixed)                \
  V(Unreachable, "unreachable", Fixed)    \
  FOR_EACH_ARITHMETIC_BINARY_OPERATION(V) \
  FOR_EACH_BITWISE_BINARY_OPERATION(V)    \
  FOR_EACH_BITWISE_SHIFT_OPERATION(V)     \
  FOR_EACH_EQUALITY_OPERATION(V)          \
  FOR_EACH_RELATIONAL_OPERATION(V)        \
  FOR_EACH_TYPE_CAST_OPERATION(V)

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
