// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_HIR_INSTRUCTIONS_FORWARD_H_

#include <string>

#include "elang/base/types.h"

namespace elang {
namespace hir {

// Forward declarations
class Block;
class Factory;
class Instruction;
class Literal;
class Operand;
class OperandVisitor;
class Type;
class TypeFactory;
class NullLiteral;  // typed null literal. |TypeFactory| singleton.
class VoidLiteral;  // operand for 'void' type. |TypeFactory| singleton.

// Visitor |V| takes three parameters:
//  Name         capitalized name for C++ class
//  name         small case name
//  num_operands Number of operands
#define FOR_EACH_HIR_INSTRUCTION(V) \
  V(Call, "call", 2)                \
  V(Entry, "entry", 0)              \
  V(Exit, "exit", 0)                \
  V(Return, "ret", 1)

// Visitor |V| takes three parameters:
//  Name     capitalized name for C++ class
//  name     small case name
//  cpp_type in C++.
#define FOR_EACH_HIR_LITERAL_OPERAND(V) \
  V(Bool, bool, bool)                   \
  V(Char, char, base::char16)           \
  V(Float32, float32, float32_t)        \
  V(Float64, float64, float64_t)        \
  V(Int16, int16, int16_t)              \
  V(Int32, int32, int32_t)              \
  V(Int64, int64, int64_t)              \
  V(Int8, int8, int8_t)                 \
  V(UInt16, uint16, uint16_t)           \
  V(UInt32, uint32, uint32_t)           \
  V(UInt64, uint64, uint64_t)           \
  V(UInt8, uint8, uint8_t)              \
  V(String, string, base::StringPiece16)

#define V(Name, ...) class Name##Literal;
FOR_EACH_HIR_LITERAL_OPERAND(V)
#undef V

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_FORWARD_H_
