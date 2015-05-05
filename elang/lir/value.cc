// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/value.h"

#include "base/logging.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

bool Value::is_memory_proxy() const {
  return is_argument() || is_parameter() || is_spill_slot();
}

bool Value::is_memory_slot() const {
  return is_stack_slot() || is_frame_slot();
}

bool Value::is_output() const {
  switch (kind) {
    case Kind::Argument:
    case Kind::Conditional:
    case Kind::FrameSlot:
    case Kind::Parameter:
    case Kind::PhysicalRegister:
    case Kind::SpillSlot:
    case Kind::StackSlot:
    case Kind::VirtualRegister:
      return true;
  }
  return false;
}

bool Value::is_void_type() const {
  return type == Type::Integer && size == ValueSize::Size0 &&
         kind == Kind::Void;
}

Value Value::Argument(Value type, int data) {
  return Value(type.type, type.size, Kind::Argument, data);
}

bool Value::CanBeImmediate(int64_t value) {
  return value >= kMinimumImmediate && value <= kMaximumImmediate;
}

Value Value::False() {
  return Value(Type::Integer, ValueSize::Size8, Kind::Conditional, 0);
}

Value Value::Float32Literal() {
  return Value(Float32Type(), Kind::Literal);
}

Value Value::Float64Literal() {
  return Value(Float64Type(), Kind::Literal);
}

Value Value::Float32Type() {
  return Value(Type::Float, ValueSize::Size32, Kind::Void, 0);
}

Value Value::Float64Type() {
  return Value(Type::Float, ValueSize::Size64, Kind::Void, 0);
}

Value Value::FrameSlot(Value type, int data) {
  return Value(type.type, type.size, Kind::FrameSlot, data);
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

Value Value::IntPtrType() {
  return Target::IntPtrType();
}

Value Value::Literal(Value type) {
  return Value(type, Kind::Literal);
}

Value Value::Parameter(Value type, int data) {
  return Value(type.type, type.size, Kind::Parameter, data);
}

Value Value::Register(Value type, int data) {
  return Value(type.type, type.size, Kind::VirtualRegister, data);
}

Value Value::SmallInt16(int data) {
  DCHECK_LT(static_cast<uint32_t>(data), 1u << 16);
  return Immediate(ValueSize::Size16, data);
}

Value Value::SmallInt32(int data) {
  return Immediate(ValueSize::Size32, data);
}

Value Value::SmallInt64(int data) {
  return Immediate(ValueSize::Size64, data);
}

Value Value::SmallInt8(int data) {
  DCHECK_LT(static_cast<uint32_t>(data), 1u << 8);
  return Immediate(ValueSize::Size8, data);
}

Value Value::SpillSlot(Value type, int data) {
  return Value(type.type, type.size, Kind::SpillSlot, data);
}

Value Value::StackSlot(Value type, int data) {
  return Value(type.type, type.size, Kind::StackSlot, data);
}

Value Value::True() {
  return Value(Type::Integer, ValueSize::Size8, Kind::Conditional, 1);
}

Value Value::TypeOf(Value value) {
  return Value(value.type, value.size, Kind::Void, 0);
}

Value Value::VoidType() {
  return Value(Type::Integer, ValueSize::Size0, Kind::Void, 0);
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
