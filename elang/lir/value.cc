// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/value.h"

#include "base/logging.h"

namespace elang {
namespace lir {

Value Value::Argument(Type type, ValueSize size, int data) {
  return Value(type, size, Kind::Argument, data);
}

bool Value::CanBeImmediate(int64_t value) {
  return value >= kMinimumImmediate && value <= kMaximumImmediate;
}

Value Value::False() {
  return Value(Type::Integer, ValueSize::Size8, Kind::Condition, 0);
}

Value Value::Float32Literal() {
  return Value(Type::Float, ValueSize::Size32, Kind::Literal);
}

Value Value::Float64Literal() {
  return Value(Type::Float, ValueSize::Size64, Kind::Literal);
}

Value Value::FloatRegister(ValueSize size, int data) {
  return Value(Type::Float, size, Kind::VirtualRegister, data);
}

Value Value::Float32Type() {
  return Value(Type::Float, ValueSize::Size32, Kind::Void);
}

Value Value::Float64Type() {
  return Value(Type::Float, ValueSize::Size64, Kind::Void);
}

Value Value::Immediate(ValueSize size, int data) {
  DCHECK(CanBeImmediate(data));
  return Value(Type::Integer, size, Kind::Immediate, data);
}

Value Value::Int16Type() {
  return Value(Type::Integer, ValueSize::Size16, Kind::Void);
}

Value Value::Int32Type() {
  return Value(Type::Integer, ValueSize::Size32, Kind::Void);
}

Value Value::Int64Type() {
  return Value(Type::Integer, ValueSize::Size64, Kind::Void);
}

Value Value::Int8Type() {
  return Value(Type::Integer, ValueSize::Size8, Kind::Void);
}

Value Value::Parameter(Type type, ValueSize size, int data) {
  return Value(type, size, Kind::Parameter, data);
}

Value Value::Register(ValueSize size, int data) {
  return Value(Type::Integer, size, Kind::VirtualRegister, data);
}

Value Value::SmallInt32(int data) {
  return Immediate(ValueSize::Size32, data);
}

Value Value::SpillSlot(Value type, int data) {
  return Value(type.type, type.size, Kind::SpillSlot, data);
}

Value Value::StackSlot(Value type, int data) {
  return Value(type.type, type.size, Kind::StackSlot, data);
}

Value Value::True() {
  return Value(Type::Integer, ValueSize::Size8, Kind::Condition, 1);
}

bool Value::is_output() const {
  switch (kind) {
    case Kind::Argument:
    case Kind::Condition:
    case Kind::Parameter:
    case Kind::PhysicalRegister:
    case Kind::SpillSlot:
    case Kind::StackSlot:
    case Kind::VirtualRegister:
      return true;
  }
  return false;
}

}  // namespace lir
}  // namespace elang

namespace std {
size_t hash<elang::lir::Value>::operator()(
    const elang::lir::Value& value) const {
  auto const p = reinterpret_cast<const uint32_t*>(&value);
  return std::hash<uint32_t>()(*p);
}
}  // namespace std
