// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_VALUES_FORWARD_H_
#define ELANG_HIR_VALUES_FORWARD_H_

#include <string>

#include "elang/base/float_types.h"

namespace elang {
namespace hir {

// Forward declarations
class BasicBlockEditor;
class Factory;
class FunctionEditor;
class Literal;
class Value;
class ValueVisitor;
class Type;
class TypeFactory;

// Visitor |V| takes three parameters:
//  Name     capitalized name for C++ class
//  name     small case name
//  cpp_type in C++.
#define FOR_EACH_HIR_LITERAL_VALUE(V)    \
  V(Bool, bool, bool)                    \
  V(Char, char, base::char16)            \
  V(Float32, float32, float32_t)         \
  V(Float64, float64, float64_t)         \
  V(Int16, int16, int16_t)               \
  V(Int32, int32, int32_t)               \
  V(Int64, int64, int64_t)               \
  V(Int8, int8, int8_t)                  \
  V(String, string, base::StringPiece16) \
  V(UInt16, uint16, uint16_t)            \
  V(UInt32, uint32, uint32_t)            \
  V(UInt64, uint64, uint64_t)            \
  V(UInt8, uint8, uint8_t)

#define V(Name, ...) class Name##Literal;
FOR_EACH_HIR_LITERAL_VALUE(V)
#undef V

// List of concrete C++ classes representing HIR values other than
// instruction.
#define FOR_EACH_HIR_VALUE(V) \
  V(BasicBlock)               \
  V(Function)                 \
  V(BoolLiteral)              \
  V(CharLiteral)              \
  V(Float32Literal)           \
  V(Float64Literal)           \
  V(Int16Literal)             \
  V(Int32Literal)             \
  V(Int64Literal)             \
  V(Int8Literal)              \
  V(NullLiteral)              \
  V(Reference)                \
  V(StringLiteral)            \
  V(UInt16Literal)            \
  V(UInt32Literal)            \
  V(UInt64Literal)            \
  V(UInt8Literal)             \
  V(VoidLiteral)

#define V(Name, ...) class Name;
FOR_EACH_HIR_VALUE(V)
#undef V

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_VALUES_FORWARD_H_
