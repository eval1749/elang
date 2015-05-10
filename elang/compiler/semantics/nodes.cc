// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/compiler/semantics/nodes.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/parameter_kind.h"

namespace elang {
namespace compiler {
namespace sm {

namespace {

Signature::Arity ComputeArity(const std::vector<Parameter*>& parameters) {
  Signature::Arity arity;
  arity.maximum = 0;
  arity.minimum = 0;
  arity.is_rest = false;
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
        arity.is_rest = true;
        break;
    }
  }
  return arity;
}

}  // namespace

// ArrayType
ArrayType::ArrayType(Zone* zone,
                     Type* element_type,
                     const std::vector<int>& dimensions)
    : Type(element_type->token()),
      dimensions_(zone, dimensions),
      element_type_(element_type) {
  DCHECK(!dimensions_.empty());
#ifndef NDEBUG
  for (auto const dimension : dimensions_)
    DCHECK_GE(dimension, -1);
#endif
}

bool ArrayType::IsSubtypeOf(const Type* other) const {
  if (this == other)
    return true;
  auto const array_type = other->as<ArrayType>();
  if (!array_type)
    return false;
  return dimensions_.size() == array_type->dimensions_.size() &&
         element_type_->IsSubtypeOf(array_type->element_type_);
}

// Class
Class::Class(Zone* zone, Semantic* outer, Token* name, ast::Class* ast_class)
    : NamedMember(outer, name),
      ast_class_(ast_class),
      base_classes_(zone),
      direct_base_classes_(zone),
      has_base_(false),
      members_(zone) {
  DCHECK(outer->is<Class>() || outer->is<Namespace>()) << outer << " " << name;
}

const ZoneUnorderedSet<Class*>& Class::base_classes() const {
  DCHECK(has_base()) << this;
  return base_classes_;
}

const ZoneVector<Class*>& Class::direct_base_classes() const {
  DCHECK(has_base()) << this;
  return direct_base_classes_;
}

bool Class::is_class() const {
  return ast_class_->is_class();
}

Semantic* Class::FindMember(Token* name) const {
  auto const it = members_.find(name->atomic_string());
  return it == members_.end() ? nullptr : it->second;
}

// Type
bool Class::IsSubtypeOf(const Type* other) const {
  if (this == other)
    return true;
  auto const other_class = other->as<Class>();
  if (!other_class)
    return nullptr;
  return !!other_class->base_classes().count(const_cast<Class*>(this));
}

// Enum
Enum::Enum(Zone* zone, Semantic* outer, Token* name)
    : NamedMember(outer, name), enum_base_(nullptr), members_(zone) {
  DCHECK(outer->is<Class>() || outer->is<Namespace>()) << outer << " " << name;
}

Type* Enum::enum_base() const {
  DCHECK(enum_base_) << *this;
  return enum_base_;
}

bool Enum::IsSubtypeOf(const Type* other) const {
  return this == other;
}

// EnumMember
EnumMember::EnumMember(Enum* owner, Token* name)
    : NamedMember(owner, name), value_(nullptr) {
}

Value* EnumMember::value() const {
  DCHECK(value_) << *this;
  return value_;
}

// InvalidValue
InvalidValue::InvalidValue(Type* type, Token* token) : Value(type, token) {
}

// Literal
Literal::Literal(Type* type, Token* token) : Value(type, token), data_(token) {
}

// Method
Method::Method(MethodGroup* method_group,
               Signature* signature,
               ast::Method* ast_method)
    : NamedMember(method_group->outer(), method_group->name()),
      ast_method_(ast_method),
      method_group_(method_group),
      signature_(signature) {
}

const ZoneVector<Parameter*>& Method::parameters() const {
  return signature_->parameters();
}

Type* Method::return_type() const {
  return signature_->return_type();
}

// MethodGroup
MethodGroup::MethodGroup(Zone* zone, Class* owner, Token* name)
    : NamedMember(owner, name), methods_(zone) {
}

// Namespace
Namespace::Namespace(Zone* zone, Namespace* outer, Token* name)
    : NamedMember(outer, name), members_(zone) {
}

Semantic* Namespace::FindMember(Token* name) const {
  auto const it = members_.find(name->atomic_string());
  return it == members_.end() ? nullptr : it->second;
}

// Parameter
Parameter::Parameter(ast::Parameter* ast_parameter,
                     Type* type,
                     Value* default_value)
    : Semantic(ast_parameter->token()),
      ast_parameter_(ast_parameter),
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

// Semantic
Semantic::Semantic(Token* token) : token_(token) {
}

Token* Semantic::name() const {
  NOTREACHED() << *this;
  return nullptr;
}

Semantic* Semantic::outer() const {
  NOTREACHED() << *this;
  return nullptr;
}

// Signature
Signature::Signature(Zone* zone,
                     Type* return_type,
                     const std::vector<Parameter*>& parameters)
    : Type(return_type->token()),
      arity_(ComputeArity(parameters)),
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
Type::Type(Token* token) : Semantic(token) {
}

// UndefinedType
UndefinedType::UndefinedType(Token* token) : Type(token) {
}

bool UndefinedType::IsSubtypeOf(const Type* other) const {
  return false;
}

// Variable
Variable::Variable(Type* type, StorageClass storage, ast::NamedNode* ast_node)
    : Semantic(ast_node->name()),
      ast_node_(ast_node),
      storage_(storage),
      type_(type) {
}

// Value
Value::Value(Type* type, Token* token) : Semantic(token), type_(type) {
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
