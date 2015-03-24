// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/optimizer/types.h"

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/type_visitor.h"

namespace elang {
namespace optimizer {

#define V(Name) \
  void Name::Accept(TypeVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_OPTIMIZER_CONCRETE_TYPE(V)
#undef V

// ArrayType
ArrayType::ArrayType(Zone* zone,
                     Type* element_type,
                     const std::vector<int>& dimensions)
    : dimensions_(zone, dimensions),
      element_type_(element_type),
      null_literal_(new (zone) NullNode(this)) {
#if _DEBUG
  for (auto const dimension : dimensions) {
    // dimension == -1 means unbound dimension.
    DCHECK_GE(dimension, -1);
  }
#endif
}

Node* ArrayType::default_value() const {
  DCHECK(null_literal_);
  return null_literal_;
}

// ControlType
ControlType::ControlType() {
}

// EffectType
EffectType::EffectType() {
}

// ExternalType
ExternalType::ExternalType(Zone* zone, AtomicString* name)
    : ReferenceType(zone, name) {
}

// FunctionType
FunctionType::FunctionType(Type* return_type, Type* parameters_type)
    : parameters_type_(parameters_type), return_type_(return_type) {
}

// PointerType
PointerType::PointerType(Zone* zone, Type* pointee)
    : null_literal_(new (zone) NullNode(this)), pointee_(pointee) {
}

Node* PointerType::default_value() const {
  DCHECK(null_literal_);
  return null_literal_;
}

Type::RegisterClass PointerType::register_class() const {
  return RegisterClass::General;
}

// PrimitiveTypes
// TODO(eval1749) NYI literal object cache.
#define V(Name, name, data_type, bit_size_value, kind_value, signedness_value) \
  PrimitiveType::RegisterClass Name##Type::register_class() const {            \
    return kind_value;                                                         \
  }                                                                            \
  int Name##Type::bit_size() const { return bit_size_value; }                  \
  Signedness Name##Type::signedness() const {                                  \
    return Signedness::signedness_value;                                       \
  }                                                                            \
                                                                               \
  /* Constructor of primitive type */                                          \
  Name##Type::Name##Type(Zone* zone)                                           \
      : default_value_(new (zone) Name##Node(this, 0)) {}                      \
                                                                               \
  /* Type */                                                                   \
  Node* Name##Type::default_value() const { return default_value_; }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

// ReferenceType
ReferenceType::ReferenceType(Zone* zone, AtomicString* name)
    : name_(name), null_literal_(new (zone) NullNode(this)) {
}

Node* ReferenceType::default_value() const {
  DCHECK(null_literal_);
  return null_literal_;
}

Type::RegisterClass ReferenceType::register_class() const {
  return RegisterClass::General;
}

// StringType
StringType::StringType(Zone* zone, AtomicString* name)
    : ReferenceType(zone, name) {
}

// TupleType
TupleType::TupleType(Zone* zone, const std::vector<Type*>& components)
    : components_(zone, components) {
  DCHECK_GE(components.size(), 2u);
}

// Type
bool Type::can_allocate_on_stack() const {
  return register_class() == RegisterClass::General ||
         register_class() == RegisterClass::Integer ||
         register_class() == RegisterClass::Float ||
         register_class() == RegisterClass::Tuple;
}

Node* Type::default_value() const {
  NOTREACHED() << *this << " doesn't have default value.";
  return nullptr;
}

Type::RegisterClass Type::register_class() const {
  return RegisterClass::Void;
}

// VoidType
VoidType::VoidType(Zone* zone) : default_value_(new (zone) VoidNode(this)) {
}

int VoidType::bit_size() const {
  return 0;
}

Node* VoidType::default_value() const {
  return default_value_;
}

}  // namespace optimizer
}  // namespace elang
