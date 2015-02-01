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
  enum class Kind : uint32_t {
    Invalid,
    FloatRegister,
    GeneralRegister,
    Immediate,
    Literal,
    VirtualGeneralRegister,
    VirtualFloatRegister,
    NotUsed7,
  };

  static const int kMaximumImmediate = 1 << 28;
  static const int kMinimumImmediate = -1 << 28;

  Kind kind : 3;
  int data : 29;

  Value() : kind(Kind::Invalid), data(0) {}
  Value(Kind kind, int data) : kind(kind), data(data) {}

  bool is_register() const {
    return kind == Kind::FloatRegister || kind == Kind::GeneralRegister ||
           kind == Kind::VirtualFloatRegister ||
           kind == Kind::VirtualGeneralRegister;
  }

  static bool CanBeImmediate(int value) {
    return value >= kMinimumImmediate && value <= kMaximumImmediate;
  }

  static bool CanBeImmediate(bool value) = delete;
  static bool CanBeImmediate(float32_t value) = delete;
  static bool CanBeImmediate(float64_t value) = delete;
  static bool CanBeImmediate(int64_t value) = delete;
  static bool CanBeImmediate(uint32_t value) = delete;
  static bool CanBeImmediate(uint64_t value) = delete;
};

static_assert(sizeof(Value) == sizeof(int),
              "Value must be packed into 32bit integer.");

inline bool operator==(const Value& value1, const Value& value2) {
  return value1.kind == value2.kind && value1.data == value2.data;
}

inline bool operator!=(const Value& value1, const Value& value2) {
  return !operator==(value1, value2);
}

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value& value);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value::Kind& kind);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_VALUE_H_
