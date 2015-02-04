// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_VALUE_H_
#define ELANG_LIR_VALUE_H_

#include <stdint.h>
#include <ostream>

#include "elang/base/float_types.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Value represents both input and output operand of instruction.
//
struct ELANG_LIR_EXPORT Value {
  enum class Type : uint32_t {
    Integer,
    Float,
  };

  enum class Size : uint32_t {
    Size8,
    Size16,
    Size32,
    Size64,
  };

  enum class Kind : uint32_t {
    Void = 0,
    Immediate = 1,
    Literal = 2,
    Parameter = 3,
    PhysicalRegister = 4,
    VirtualRegister = 5,
    PseudoRegister = 6,  // for x64 EFLAGS
    Argument = 7,
    NotUsed8,
    NotUsed9,
    NotUsed10,
    NotUsed11,
    NotUsed12,
    NotUsed13,
    NotUsed14,
    Instruction,  // for ErrorData
  };

  static const int kMaximumImmediate = 1 << 23;
  static const int kMinimumImmediate = -1 << 23;

  Type type : 1;
  Size size : 3;
  Kind kind : 4;
  int data : 24;

  Value() : type(Type::Integer), size(Size::Size8), kind(Kind::Void), data(0) {}

  Value(Type type, Size size, Kind kind, int data)
      : type(type), size(size), kind(kind), data(data) {}

  Value(Type type, Size size, Kind kind)
      : type(type), size(size), kind(kind), data(0) {}

  // predicates for |Type|
  bool is_float() const { return type == Type::Float; }
  bool is_integer() const { return type == Type::Integer; }

  // predicates for |Kind|
  bool is_immediate() const { return kind == Kind::Immediate; }
  bool is_literal() const { return kind == Kind::Literal; }
  bool is_register() const { return is_physical() || is_virtual(); }
  bool is_physical() const { return kind == Kind::PhysicalRegister; }
  bool is_read_only() const { return is_immediate() || is_literal(); }
  bool is_virtual() const { return kind == Kind::VirtualRegister; }
  bool is_void() const { return kind == Kind::Void; }

  // Help function
  static bool CanBeImmediate(int64_t value);

  static Value Argument(Type type, Size size, int data);
  static Value Float32Literal();
  static Value Float64Literal();
  static Value FloatRegister(Size size, int data);
  static Value Immediate(Size size, int data);
  static Value Parameter(Type type, Size size, int data);
  static Value Register(Size size, int data);
};

static_assert(sizeof(Value) == sizeof(int),
              "Value must be packed into 32bit integer.");

inline bool operator==(const Value& value1, const Value& value2) {
  return value1.data == value2.data && value1.kind == value2.kind &&
         value1.size == value2.size && value1.type == value2.type;
}

inline bool operator!=(const Value& value1, const Value& value2) {
  return !operator==(value1, value2);
}

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value& value);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value::Kind& kind);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value::Size& size);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value::Type& type);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_VALUE_H_
