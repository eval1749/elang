// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPES_FORWARD_H_
#define ELANG_HIR_TYPES_FORWARD_H_

#include "elang/base/types.h"

namespace elang {
namespace hir {

// Forward declarations
class PoinerType;
class ReferenceType;
class StringType;
class Type;
class TypeFactory;

// Note: When you add primitive type, you should also update
// |FOR_EACH_HIR_LITERAL_OPERAND|.
//
// Visitor |V| takes following parameters:
//  Name        capitalized name
//  name        small case name
//  data_type  C++ type for |NewLiteral(Factory*, data_type)|.
//  bit_size    number of bits
//  kind        kind of register to hold the value.
#define FOR_EACH_HIR_PRIMITIVE_TYPE(V)      \
  V(Bool, bool, bool, 1, General)           \
  V(Char, char, base::char16, 16, General)  \
  V(Float32, float32, float32_t, 32, Float) \
  V(Float64, float64, float64_t, 64, Float) \
  V(Int16, int16, int16_t, 16, General)     \
  V(Int32, int32, int32_t, 32, General)     \
  V(Int64, int64, int64_t, 64, General)     \
  V(Int8, int8, int8_t, 8, General)         \
  V(UInt16, uint16, uint16_t, 16, General)  \
  V(UInt32, uint32, uint32_t, 32, General)  \
  V(UInt64, uint64, uint64_t, 64, General)  \
  V(UInt8, uint8, uint8_t, 8, General)      \
  V(Void, void, int, 0, Void)

#define V(Name, ...) class Name##Type;
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPES_FORWARD_H_
