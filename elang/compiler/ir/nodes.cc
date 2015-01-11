// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ir/nodes.h"

#include "elang/compiler/ast/class.h"

namespace elang {
namespace compiler {
namespace ir {

Node::Node() {
}

Class::Class(Zone* zone, ast::Class* ast_class,
             const std::vector<Class*>& base_classes)
    : ast_class_(ast_class), base_classes_(zone, base_classes) {
}

bool Class::is_class() const {
  return ast_class_->is_class();
}

Enum::Enum(Zone* zone, ast::Enum* ast_enum, const std::vector<int64_t>& values)
    : ast_enum_(ast_enum), values_(zone, values) {
}

Parameter::Parameter(Token* name, Type* type, Value* default_value)
    : default_value_(default_value), name_(name), type_(type) {
}

Signature::Signature(Zone* zone,
                     Token* name,
                     Type* return_type,
                     const std::vector<Parameter*>& parameters)
    : name_(name), parameters_(zone, parameters), return_type_(return_type) {
}

Type::Type() {
}

}  // namespace ir
}  // namespace compiler
}  // namespace elang
