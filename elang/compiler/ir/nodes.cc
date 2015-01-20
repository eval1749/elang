// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/compiler/ir/nodes.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/parameter_kind.h"

namespace elang {
namespace compiler {
namespace ir {

namespace {

void ComputeBaseClasses(Class* clazz, ZoneUnorderedSet<Class*>* classes) {
  if (classes->count(clazz))
    return;
  classes->insert(clazz);
  for (auto const base_class : clazz->direct_base_classes())
    ComputeBaseClasses(base_class, classes);
}

Signature::Arity ComputeArity(const std::vector<Parameter*>& parameters) {
  Signature::Arity arity;
  arity.maximum = 0;
  arity.minimum = 0;
  auto state = ParameterKind::Required;
  for (auto const parameter : parameters) {
    switch (parameter->kind()) {
      case ParameterKind::Required:
        DCHECK_EQ(ParameterKind::Required, state);
        ++arity.minimum;
        ++arity.maximum;
        break;
      case ParameterKind::Optional:
        DCHECK(state == ParameterKind::Required ||
               state == ParameterKind::Optional);
        state = ParameterKind::Optional;
        ++arity.maximum;
        break;
      case ParameterKind::Rest:
        DCHECK(state == ParameterKind::Required ||
               state == ParameterKind::Optional);
        state = ParameterKind::Rest;
        arity.maximum = Signature::Arity::kMaximum;
        break;
    }
  }
  return arity;
}

}  // namespace

// Class
Class::Class(Zone* zone,
             ast::Class* ast_class,
             const std::vector<Class*>& direct_base_classes)
    : ast_class_(ast_class),
      base_classes_(zone),
      direct_base_classes_(zone, direct_base_classes) {
  for (auto const base_class : direct_base_classes)
    ComputeBaseClasses(base_class, &base_classes_);
}

bool Class::is_class() const {
  return ast_class_->is_class();
}

bool Class::IsSubtypeOf(const Type* other) const {
  if (this == other)
    return true;
  auto const other_class = other->as<Class>();
  if (!other_class)
    return nullptr;
  return !!other_class->base_classes().count(const_cast<Class*>(this));
}

// Enum
Enum::Enum(Zone* zone,
           ast::Enum* ast_enum,
           ir::Class* base_type,
           const std::vector<int64_t>& values)
    : ast_enum_(ast_enum), base_type_(base_type), values_(zone, values) {
}

bool Enum::IsSubtypeOf(const Type* other) const {
  return this == other;
}

// Literal
Literal::Literal(Type* type, Token* token) : data_(token), type_(type) {
}

// Method
Method::Method(ast::Method* ast_method, Signature* signature)
    : ast_method_(ast_method), signature_(signature) {
}

const ZoneVector<Parameter*>& Method::parameters() const {
  return signature_->parameters();
}

Type* Method::return_type() const {
  return signature_->return_type();
}

// Node
Node::Node() {
}

// Parameter
Parameter::Parameter(ast::Parameter* ast_parameter,
                     Type* type,
                     Value* default_value)
    : ast_parameter_(ast_parameter),
      default_value_(default_value),
      type_(type) {
}

bool Parameter::operator==(const Parameter& other) const {
  if (this == &other)
    return true;
  return kind() == other.kind() && name() == other.name() &&
         default_value_ == other.default_value_;
}

bool Parameter::operator!=(const Parameter& other) const {
  return !operator==(other);
}

bool Parameter::is_rest() const {
  return kind() == ParameterKind::Rest;
}

ParameterKind Parameter::kind() const {
  return ast_parameter_->kind();
}

Token* Parameter::name() const {
  return ast_parameter_->name();
}

int Parameter::position() const {
  return ast_parameter_->position();
}

bool Parameter::IsIdentical(const Parameter& other) const {
  return type_ == other.type_;
}

// Signature
Signature::Signature(Zone* zone,
                     Type* return_type,
                     const std::vector<Parameter*>& parameters)
    : arity_(ComputeArity(parameters)),
      parameters_(zone, parameters),
      return_type_(return_type) {
}

bool Signature::operator==(const Signature& other) const {
  if (this == &other)
    return true;
  return return_type_ == other.return_type_ && parameters_ == other.parameters_;
}

bool Signature::operator!=(const Signature& other) const {
  return !operator==(other);
}

bool Signature::IsIdenticalParameters(const Signature* other) const {
  if (this == other)
    return true;
  if (parameters_.size() != other->parameters_.size())
    return false;
  auto other_parameters = other->parameters_.begin();
  for (auto const parameter : parameters_) {
    if (!parameter->IsIdentical(**other_parameters))
      return false;
    ++other_parameters;
  }
  return true;
}

// Type
bool Signature::IsSubtypeOf(const Type* other) const {
  if (this == other)
    return true;
  return false;
}

// Type
Type::Type() {
}

}  // namespace ir
}  // namespace compiler
}  // namespace elang
