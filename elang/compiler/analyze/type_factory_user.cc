// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/type_factory_user.h"

#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"

namespace elang {
namespace compiler {
namespace ts {

FactoryUser::FactoryUser(Factory* factory)
    : factory_(factory) {
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

ts::Value* FactoryUser::NewInvalidValue(ast::Node* node) {
  return factory()->NewInvalidValue(node);
}

ts::Value* FactoryUser::NewLiteral(ir::Type* type) {
  return factory()->NewLiteral(type);
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
