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
    : dimensions_(zone, dimensions), element_type_(element_type) {
#if _DEBUG
  for (auto const dimension : dimensions) {
    // dimension == -1 means unbound dimension.
    DCHECK_GE(dimension, -1);
  }
#endif
}

// ControlType
ControlType::ControlType(Type* data_type) : data_type_(data_type) {
}

// EffectType
EffectType::EffectType() {
}

// ExternalType
ExternalType::ExternalType(AtomicString* name) : ReferenceType(name) {
}

// FunctionType
FunctionType::FunctionType(Type* return_type, Type* parameters_type)
    : parameters_type_(parameters_type), return_type_(return_type) {
}

// PointerType
PointerType::PointerType(Type* pointee) : pointee_(pointee) {
}

Type::RegisterClass PointerType::register_class() const {
  return RegisterClass::General;
}

// PrimitiveValueTypes
#define V(Name, name, data_type, bit_size_value, kind_value, signedness_value) \
  /* Constructor of primitive type */                                          \
  Name##Type::Name##Type() {}                                                  \
  PrimitiveType::RegisterClass Name##Type::register_class() const {            \
    return kind_value;                                                         \
  }                                                                            \
  int Name##Type::bit_size() const { return bit_size_value; }                  \
  Signedness Name##Type::signedness() const {                                  \
    return Signedness::signedness_value;                                       \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

// ReferenceType
ReferenceType::ReferenceType(AtomicString* name) : name_(name) {
}

Type::RegisterClass ReferenceType::register_class() const {
  return RegisterClass::General;
}

// StringType
StringType::StringType(AtomicString* name) : ReferenceType(name) {
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

Type::RegisterClass Type::register_class() const {
  return RegisterClass::Void;
}

Signedness Type::signedness() const {
  return Signedness::Unsigned;
}

// VoidType
VoidType::VoidType() {
}

int VoidType::bit_size() const {
  return 0;
}

}  // namespace optimizer
}  // namespace elang
