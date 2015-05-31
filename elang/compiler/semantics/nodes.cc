// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "base/logging.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token.h"

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
Class::Class(Zone* zone,
             Semantic* outer,
             Kind kind,
             Modifiers modifiers,
             Token* name)
    : NamedMember(outer, name),
      WithModifiers(modifiers),
      base_classes_(zone),
      direct_base_classes_(zone),
      has_base_(false),
      kind_(kind),
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

Semantic* Class::FindMemberByString(AtomicString* name) const {
  auto const it = members_.find(name);
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

// Const
Const::Const(Class* owner, Token* name)
    : NamedMember(owner, name), type_(nullptr), value_(nullptr) {
}

Class* Const::owner() const {
  return outer()->as<Class>();
}

Type* Const::type() const {
  DCHECK(type_) << *this;
  return type_;
}

Value* Const::value() const {
  DCHECK(value_) << *this;
  return value_;
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

Semantic* Enum::FindMemberByString(AtomicString* name) const {
  auto const it = std::find_if(members_.begin(), members_.end(),
                               [name](const EnumMember* member) {
                                 return member->name()->atomic_string() == name;
                               });
  return it == members_.end() ? nullptr : *it;
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

// Field
Field::Field(Class* owner, Token* name)
    : NamedMember(owner, name), value_(nullptr) {
}

Class* Field::owner() const {
  return outer()->as<Class>();
}

// InvalidValue
InvalidValue::InvalidValue(Type* type, Token* token) : Value(type, token) {
}

// Literal
Literal::Literal(Type* type, Token* token) : Value(type, token), data_(token) {
}

// Method
Method::Method(MethodGroup* method_group,
               Modifiers modifiers,
               Signature* signature)
    : NamedMember(method_group->outer(), method_group->name()),
      method_group_(method_group),
      modifiers_(modifiers),
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

Semantic* Namespace::FindMemberByString(AtomicString* name) const {
  auto const it = members_.find(name);
  return it == members_.end() ? nullptr : it->second;
}

// Parameter
Parameter::Parameter(ParameterKind kind,
                     int position,
                     Type* type,
                     Token* name,
                     Value* default_value)
    : Semantic(name),
      default_value_(default_value),
      kind_(kind),
      name_(name),
      position_(position),
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

Token* Parameter::name() const {
  return name_;
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

Semantic* Semantic::FindMember(AtomicString* name) const {
  return FindMemberByString(name);
}

Semantic* Semantic::FindMember(Token* name) const {
  return FindMemberByString(name->atomic_string());
}

Semantic* Semantic::FindMemberByString(AtomicString* name) const {
  return nullptr;
}

bool Semantic::IsDescendantOf(const Semantic* other) const {
  DCHECK(other);
  for (auto runner = outer(); runner; runner = runner->outer()) {
    if (runner == other)
      return true;
  }
  return false;
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
Variable::Variable(Type* type, StorageClass storage, Token* name)
    : Semantic(name), name_(name), storage_(storage), type_(type) {
}

Token* Variable::name() const {
  return name_;
}

// Value
Value::Value(Type* type, Token* token) : Semantic(token), type_(type) {
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
