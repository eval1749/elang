// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/value.h"

#include "base/logging.h"

namespace elang {
namespace lir {

bool Value::is_memory_slot() const {
  return is_parameter() || is_argument() || is_stack_slot() || is_spill_slot();
}

Value Value::Argument(Value type, int data) {
  return Value(type.type, type.size, Kind::Argument, data);
}

bool Value::CanBeImmediate(int64_t value) {
  return value >= kMinimumImmediate && value <= kMaximumImmediate;
}

Value Value::False() {
  return Value(Type::Integer, ValueSize::Size8, Kind::Condition, 0);
}

Value Value::Float32Literal() {
  return Value(Float32Type(), Kind::Literal);
}

Value Value::Float64Literal() {
  return Value(Float64Type(), Kind::Literal);
}

Value Value::FloatRegister(ValueSize size, int data) {
  return Value(Type::Float, size, Kind::VirtualRegister, data);
}

Value Value::Float32Type() {
  return Value(Type::Float, ValueSize::Size32, Kind::Void, 0);
}

Value Value::Float64Type() {
  return Value(Type::Float, ValueSize::Size64, Kind::Void, 0);
}

Value Value::Immediate(ValueSize size, int data) {
  DCHECK(CanBeImmediate(data));
  return Value(Type::Integer, size, Kind::Immediate, data);
}

Value Value::Int16Type() {
  return Value(Type::Integer, ValueSize::Size16, Kind::Void, 0);
}

Value Value::Int32Type() {
  return Value(Type::Integer, ValueSize::Size32, Kind::Void, 0);
}

Value Value::Int64Type() {
  return Value(Type::Integer, ValueSize::Size64, Kind::Void, 0);
}

Value Value::Int8Type() {
  return Value(Type::Integer, ValueSize::Size8, Kind::Void, 0);
}

Value Value::Literal(Value type) {
  return Value(type, Kind::Literal);
}

Value Value::Parameter(Value type, int data) {
  return Value(type.type, type.size, Kind::Parameter, data);
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
