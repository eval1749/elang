// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/type_values.h"

#include "base/logging.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/ir/nodes.h"

namespace elang {
namespace compiler {
namespace ts {

// AnyValue
AnyValue::AnyValue() {
}

bool AnyValue::Contains(const Value* other) const {
  DCHECK(other);
  return true;
}

// EmptyValue
EmptyValue::EmptyValue() {
}

bool EmptyValue::Contains(const Value* other) const {
  DCHECK(other);
  return false;
}

// InvalidValue
InvalidValue::InvalidValue(ast::Node* node) : node_(node) {
}

bool InvalidValue::Contains(const Value* other) const {
  DCHECK(other);
  return false;
}

// Literal
Literal::Literal(ir::Type* value) : value_(value) {
}

// Value
bool Literal::Contains(const Value* other) const {
  if (this == other)
    return true;
  if (other->is<AnyValue>())
    return false;
  if (other->is<EmptyValue>())
    return true;
  if (auto const other_null = other->as<NullValue>())
    return Contains(other_null->value());
  if (auto const other_literal = other->as<Literal>())
    return other_literal->value()->IsSubtypeOf(value());
  NOTREACHED();
  return false;
}

// NullValue
NullValue::NullValue(Value* value) : value_(value) {
}

// Value
bool NullValue::Contains(const Value* other) const {
  return value_->Contains(other);
}

// Value
Value::Value() {
}

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
  if (value.is<AnyValue>())
    return ostream << "any";
  if (value.is<EmptyValue>())
    return ostream << "empty";
  if (auto const null_value = value.as<NullValue>())
    return ostream << "null " << *null_value->value();
  if (auto const literal = value.as<Literal>())
    return ostream << *literal->value();
  if (auto const variable = value.as<Variable>())
    return ostream << "Var(" << *variable->node() << ")";
  return ostream << "Unsupported " << &value;
}

// Variable
Variable::Variable(ast::Node* node, Value* value) : node_(node), value_(value) {
}

// Variable
bool Variable::Contains(const Value* other) const {
  return value_->Contains(other);
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
