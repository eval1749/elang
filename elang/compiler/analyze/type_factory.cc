// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"

namespace elang {
namespace compiler {
namespace ts {

Factory::Factory()
    : any_value_(new (zone()) AnyValue),
      empty_value_(new (zone()) EmptyValue()) {
}

Factory::~Factory() {
}

InvalidValue* Factory::NewInvalidValue(ast::Node* node) {
  return new (zone()) InvalidValue(node);
}

Literal* Factory::NewLiteral(ir::Type* type) {
  return new (zone()) Literal(type);
}

NullValue* Factory::NewNullValue(Value* value) {
  return new (zone()) NullValue(value);
}

Variable* Factory::NewVariable(ast::Node* node) {
  return new (zone()) Variable(node);
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
