// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/type_factory_user.h"

#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace optimizer {

TypeFactoryUser::TypeFactoryUser(TypeFactory* type_factory)
    : type_factory_(type_factory) {
}

TypeFactoryUser::~TypeFactoryUser() {
}

Type* TypeFactoryUser::control_type() const {
  return type_factory_->control_type();
}

Type* TypeFactoryUser::effect_type() const {
  return type_factory_->effect_type();
}

Type* TypeFactoryUser::string_type() const {
  return type_factory_->string_type();
}

#define V(Name, name, ...)                     \
  Type* TypeFactoryUser::name##_type() const { \
    return type_factory_->name##_type();       \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V

ArrayType* TypeFactoryUser::NewArrayType(Type* element_type,
                                         const std::vector<int>& dimensions) {
  return type_factory_->NewArrayType(element_type, dimensions);
}

ControlType* TypeFactoryUser::NewControlType(Type* data_type) {
  return type_factory_->NewControlType(data_type);
}

ExternalType* TypeFactoryUser::NewExternalType(AtomicString* name) {
  return type_factory_->NewExternalType(name);
}

FunctionType* TypeFactoryUser::NewFunctionType(Type* return_type,
                                               Type* parameters_type) {
  return type_factory_->NewFunctionType(return_type, parameters_type);
}

PointerType* TypeFactoryUser::NewPointerType(Type* pointee) {
  return type_factory_->NewPointerType(pointee);
}

TupleType* TypeFactoryUser::NewTupleType(const std::vector<Type*>& components) {
  return type_factory_->NewTupleType(components);
}

}  // namespace optimizer
}  // namespace elang
