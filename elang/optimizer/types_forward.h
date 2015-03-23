// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TYPES_FORWARD_H_
#define ELANG_OPTIMIZER_TYPES_FORWARD_H_

#include <ostream>

#include "elang/base/float_types.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

// List of abstract C++ classes representing OPTIMIZER type.
#define FOR_EACH_OPTIMIZER_ABSTRACT_TYPE(V) \
  V(PrimitiveType)                          \
  V(PrimitiveValueType)                     \
  V(ReferenceType)                          \
  V(Type)

// List of concrete C++ classes representing OPTIMIZER type.
#define FOR_EACH_OPTIMIZER_CONCRETE_TYPE(V) \
  V(ArrayType)                              \
  V(BoolType)                               \
  V(CharType)                               \
  V(ControlType)                            \
  V(EffectType)                             \
  V(Float32Type)                            \
  V(Float64Type)                            \
  V(ExternalType)                           \
  V(FunctionType)                           \
  V(Int16Type)                              \
  V(Int32Type)                              \
  V(Int64Type)                              \
  V(Int8Type)                               \
  V(IntPtrType)                             \
  V(PointerType)                            \
  V(TupleType)                              \
  V(UInt16Type)                             \
  V(UInt32Type)                             \
  V(UInt64Type)                             \
  V(UInt8Type)                              \
  V(UIntPtrType)                            \
  V(StringType)                             \
  V(VoidType)

// Note: When you add primitive type, you should also update
// |FOR_EACH_OPTIMIZER_LITERAL_VALUE|.
//
// Visitor |V| takes following parameters:
//  Name        capitalized name
//  name        small case name
//  data_type  C++ type for |NewLiteral(Factory*, data_type)|.
//  bit_size    number of bits
//  kind        kind of register to hold the value.
#define FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)   \
  V(Bool, bool, bool, 1, General, Unsigned)          \
  V(Char, char, base::char16, 16, General, Unsigned) \
  V(Float32, float32, float32_t, 32, Float, Signed)  \
  V(Float64, float64, float64_t, 64, Float, Signed)  \
  V(Int16, int16, int16_t, 16, Integer, Signed)      \
  V(Int32, int32, int32_t, 32, Integer, Signed)      \
  V(Int64, int64, int64_t, 64, Integer, Signed)      \
  V(Int8, int8, int8_t, 8, Integer, Signed)          \
  V(IntPtr, intptr, int64_t, 0, Integer, Signed)     \
  V(UInt16, uint16, uint16_t, 16, Integer, Unsigned) \
  V(UInt32, uint32, uint32_t, 32, Integer, Unsigned) \
  V(UInt64, uint64, uint64_t, 64, Integer, Unsigned) \
  V(UInt8, uint8, uint8_t, 8, Integer, Unsigned)     \
  V(UIntPtr, uintptr, uint64_t, 0, Integer, Unsigned)

#define FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V) \
  FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V) \
  V(Void, void, int, 0, Void)

#define V(Name) class Name;
FOR_EACH_OPTIMIZER_ABSTRACT_TYPE(V)
FOR_EACH_OPTIMIZER_CONCRETE_TYPE(V)
#undef V

// Forward declarations
enum class Signedness;
class Thing;
class TypeFactory;
class TypeVisitor;

// For ease of using |FOR_EACH_OPTIMIZER_PREMITIVE_TYPE(V)|
#define VoidLiteral VoidValue

// Print for formatting and debugging.
ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const Type& type);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TYPES_FORWARD_H_
