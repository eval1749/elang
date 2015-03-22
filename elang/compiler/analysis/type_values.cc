// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/type_values.h"

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
AndValue::AndValue(Zone* zone, const std::vector<UnionValue*>& union_values)
    : union_values_(zone, union_values) {
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

const ZoneVector<sm::Method*>& Argument::methods() const {
  return call_value_->methods();
}

sm::Type* Argument::valueFor(const sm::Method* method) const {
  return method->parameters()[position_]->type();
}

void Argument::SetMethods(const std::vector<sm::Method*>& methods) {
  call_value_->SetMethods(methods);
}

// UnionValue
bool Argument::CanUse(sm::Method* method, sm::Type* type) const {
  return type->IsSubtypeOf(valueFor(method));
}

// CallValue
CallValue::CallValue(Zone* zone, ast::Call* ast_call)
    : ast_call_(ast_call), methods_(zone) {
}

sm::Type* CallValue::valueFor(const sm::Method* method) const {
  return method->return_type();
}

void CallValue::SetMethods(const std::vector<sm::Method*>& methods) {
  methods_.reserve(methods.size());
  methods_.resize(0);
  for (auto const method : methods)
    methods_.push_back(method);
}

// UnionValue
bool CallValue::CanUse(sm::Method* method, sm::Type* type) const {
  return valueFor(method)->IsSubtypeOf(type);
}

// EmptyValue
EmptyValue::EmptyValue() {
}

// InvalidValue
InvalidValue::InvalidValue(ast::Node* node) : node_(node) {
}

// Literal
Literal::Literal(sm::Type* value) : value_(value) {
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

Variable* Variable::Find() const {
  if (parent_ != this)
    parent_ = parent_->Find();
  return parent_;
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
