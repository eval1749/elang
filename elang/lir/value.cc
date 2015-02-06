// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/value.h"

#include "base/logging.h"

namespace elang {
namespace lir {

Value Value::Argument(Type type, Size size, int data) {
  return Value(type, size, Kind::Argument, data);
}

bool Value::CanBeImmediate(int64_t value) {
  return value >= kMinimumImmediate && value <= kMaximumImmediate;
}

Value Value::False() {
  return Value(Type::Integer, Size::Size8, Kind::Condition, 0);
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
  DCHECK(CanBeImmediate(data));
  return Value(Type::Integer, size, Kind::Immediate, data);
}

Value Value::Parameter(Type type, Size size, int data) {
  return Value(type, size, Kind::Parameter, data);
}

Value Value::Register(Size size, int data) {
  return Value(Type::Integer, size, Kind::VirtualRegister, data);
}

Value Value::SmallInt32(int data) {
  return Immediate(Value::Size::Size32, data);
}

Value Value::True() {
  return Value(Type::Integer, Size::Size8, Kind::Condition, 1);
}

}  // namespace lir
}  // namespace elang
