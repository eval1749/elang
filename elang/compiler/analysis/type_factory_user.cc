// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/type_factory_user.h"

#include "elang/compiler/analysis/type_factory.h"
#include "elang/compiler/analysis/type_values.h"

namespace elang {
namespace compiler {
namespace ts {

FactoryUser::FactoryUser(Factory* factory) : factory_(factory) {
}

FactoryUser::~FactoryUser() {
}

Value* FactoryUser::any_value() const {
  return factory()->any_value();
}

Value* FactoryUser::bool_value() const {
  return factory()->bool_value();
}

Value* FactoryUser::empty_value() const {
  return factory()->empty_value();
}

Value* FactoryUser::float32_value() const {
  return factory()->float32_value();
}

Value* FactoryUser::float64_value() const {
  return factory()->float64_value();
}

Value* FactoryUser::int16_value() const {
  return factory()->int16_value();
}

Value* FactoryUser::int32_value() const {
  return factory()->int32_value();
}

Value* FactoryUser::int64_value() const {
  return factory()->int64_value();
}

Value* FactoryUser::int8_value() const {
  return factory()->int8_value();
}

Value* FactoryUser::uint16_value() const {
  return factory()->uint16_value();
}

Value* FactoryUser::uint32_value() const {
  return factory()->uint32_value();
}

Value* FactoryUser::uint64_value() const {
  return factory()->uint64_value();
}

Value* FactoryUser::uint8_value() const {
  return factory()->uint8_value();
}

ts::Value* FactoryUser::NewInvalidValue(ast::Node* node) {
  return factory()->NewInvalidValue(node);
}

ts::Value* FactoryUser::NewLiteral(sm::Type* type) {
  return factory()->NewLiteral(type);
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
