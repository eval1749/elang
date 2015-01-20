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

// AndValue
AndValue::AndValue(Zone* zone) : union_values_(zone) {
}

void AndValue::SetUnionValues(const std::vector<UnionValue*>& union_values) {
  DCHECK_GE(union_values.size(), 2u);
  union_values_.reserve(union_values.size());
  union_values_.resize(0);
  for (auto const union_value : union_values)
    union_values_.push_back(union_value);
}

// Argument
Argument::Argument(CallValue* call_value, int position)
    : call_value_(call_value), position_(position) {
}

const ZoneVector<ir::Method*>& Argument::methods() const {
  return call_value_->methods();
}

ir::Type* Argument::value(const ir::Method* method) const {
  return method->parameters()[position_]->type();
}

void Argument::SetMethods(const std::vector<ir::Method*>& methods) {
  call_value_->SetMethods(methods);
}

// CallValue
CallValue::CallValue(Zone* zone, ast::Call* ast_call)
    : ast_call_(ast_call), methods_(zone) {
}

ir::Type* CallValue::value(const ir::Method* method) const {
  return method->return_type();
}

void CallValue::SetMethods(const std::vector<ir::Method*>& methods) {
  methods_.reserve(methods.size());
  methods_.resize(0);
  for (auto const method : methods)
    methods_.push_back(method);
}

// EmptyValue
EmptyValue::EmptyValue() {
}

// InvalidValue
InvalidValue::InvalidValue(ast::Node* node) : node_(node) {
}

// Literal
Literal::Literal(ir::Type* value) : value_(value) {
}

// NullValue
NullValue::NullValue(Value* value) : value_(value) {
}

// UnionValue
UnionValue::UnionValue() {
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
Variable::Variable(ast::Node* node, Value* value)
    : node_(node), parent_(this), rank_(0), value_(value) {
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
