// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPES_FORWARD_H_
#define ELANG_HIR_TYPES_FORWARD_H_

#include <ostream>

#include "elang/base/float_types.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

// List of abstract C++ classes representing HIR type.
#define FOR_EACH_HIR_ABSTRACT_TYPE(V) \
  V(PrimitiveType)                    \
  V(ReferenceType)                    \
  V(Type)

// List of concrete C++ classes representing HIR type.
#define FOR_EACH_HIR_CONCRETE_TYPE(V) \
  V(ArrayType)                        \
  V(BoolType)                         \
  V(CharType)                         \
  V(Float32Type)                      \
  V(Float64Type)                      \
  V(ExternalType)                     \
  V(FunctionType)                     \
  V(Int16Type)                        \
  V(Int32Type)                        \
  V(Int64Type)                        \
  V(Int8Type)                         \
  V(PointerType)                      \
  V(TupleType)                        \
  V(UInt16Type)                       \
  V(UInt32Type)                       \
  V(UInt64Type)                       \
  V(UInt8Type)                        \
  V(StringType)                       \
  V(VoidType)

// Note: When you add primitive type, you should also update
// |FOR_EACH_HIR_LITERAL_VALUE|.
//
// Visitor |V| takes following parameters:
//  Name        capitalized name
//  name        small case name
//  data_type  C++ type for |NewLiteral(Factory*, data_type)|.
//  bit_size    number of bits
//  kind        kind of register to hold the value.
#define FOR_EACH_HIR_PRIMITIVE_VALUE_TYPE(V) \
  V(Bool, bool, bool, 1, General)            \
  V(Char, char, base::char16, 16, General)   \
  V(Float32, float32, float32_t, 32, Float)  \
  V(Float64, float64, float64_t, 64, Float)  \
  V(Int16, int16, int16_t, 16, Integer)      \
  V(Int32, int32, int32_t, 32, Integer)      \
  V(Int64, int64, int64_t, 64, Integer)      \
  V(Int8, int8, int8_t, 8, Integer)          \
  V(UInt16, uint16, uint16_t, 16, Integer)   \
  V(UInt32, uint32, uint32_t, 32, Integer)   \
  V(UInt64, uint64, uint64_t, 64, Integer)   \
  V(UInt8, uint8, uint8_t, 8, Integer)

#define FOR_EACH_HIR_PRIMITIVE_TYPE(V) \
  FOR_EACH_HIR_PRIMITIVE_VALUE_TYPE(V) \
  V(Void, void, int, 0, Void)

#define V(Name) class Name;
FOR_EACH_HIR_ABSTRACT_TYPE(V)
FOR_EACH_HIR_CONCRETE_TYPE(V)
#undef V

// Forward declarations
class Thing;
class TypeFactory;
class TypeVisitor;

// For ease of using |FOR_EACH_HIR_PREMITIVE_TYPE(V)|
#define VoidLiteral VoidValue

// Print for formatting and debugging.
ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Type& type);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPES_FORWARD_H_
