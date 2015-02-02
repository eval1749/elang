// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/value.h"

namespace elang {
namespace lir {

bool Value::CanBeImmediate(int64_t value) {
  return value >= kMinimumImmediate && value <= kMaximumImmediate;
}

Value Value::Float32Literal() {
  return Value(Type::Float, Size::Size32, Kind::Literal);
}

Value Value::Float64Literal() {
  return Value(Type::Float, Size::Size64, Kind::Literal);
}

Value Value::FloatRegister(Size size, int data) {
  return Value(Type::Float, size, Kind::VirtualRegister, data);
}

Value Value::Immediate(Size size, int data) {
  return Value(Type::Integer, size, Kind::Immediate, data);
}

Value Value::Parameter(Type type, Size size, int data) {
  return Value(type, size, Kind::Parameter, data);
}

Value Value::Register(Size size, int data) {
  return Value(Type::Integer, size, Kind::VirtualRegister, data);
}

}  // namespace lir
}  // namespace elang