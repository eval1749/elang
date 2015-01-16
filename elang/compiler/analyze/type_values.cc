// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/compiler/analyze/type_values.h"

namespace elang {
namespace compiler {
namespace ts {

// AnyValue
AnyValue::AnyValue() {
}

// EmptyValue
EmptyValue::EmptyValue() {
}

// InvalidValue
InvalidValue::InvalidValue(ast::Node* node) : node_(node) {
}

// Literal
Literal::Literal(ir::Type* value) : value_(value) {
}

// NullValue
NullValue::NullValue(Value* value) : value_(value) {
}

// Value
Value::Value() {
}

// Variable
Variable::Variable(ast::Node* node) : node_(node) {
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
