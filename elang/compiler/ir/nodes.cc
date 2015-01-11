// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/compiler/ir/nodes.h"

#include "elang/compiler/ast/class.h"

namespace elang {
namespace compiler {
namespace ir {

// Node
Node::Node() {
}

// Class
Class::Class(Zone* zone,
             ast::Class* ast_class,
             const std::vector<Class*>& base_classes)
    : ast_class_(ast_class), base_classes_(zone, base_classes) {
}

bool Class::is_class() const {
  return ast_class_->is_class();
}

Enum::Enum(Zone* zone, ast::Enum* ast_enum, const std::vector<int64_t>& values)
    : ast_enum_(ast_enum), values_(zone, values) {
}

Method::Method(ast::Method* ast_method, Signature* signature)
    : ast_method_(ast_method), signature_(signature) {
}

// Parameter
Parameter::Parameter(ParameterKind kind,
                     Token* name,
                     Type* type,
                     Value* default_value)
    : default_value_(default_value), kind_(kind), name_(name), type_(type) {
}

bool Parameter::operator==(const Parameter& other) const {
  if (this == &other)
    return true;
  return kind_ == other.kind_ && name_ == other.name_ &&
         default_value_ == other.default_value_;
}

bool Parameter::operator!=(const Parameter& other) const {
  return !operator==(other);
}

bool Parameter::IsIdentical(const Parameter& other) const {
  return type_ == other.type_;
}

// Signature
Signature::Signature(Zone* zone,
                     Type* return_type,
                     const std::vector<Parameter*>& parameters)
    : parameters_(zone, parameters), return_type_(return_type) {
}

bool Signature::operator==(const Signature& other) const {
  if (this == &other)
    return true;
  return return_type_ == other.return_type_ && parameters_ == other.parameters_;
}

bool Signature::operator!=(const Signature& other) const {
  return !operator==(other);
}

bool Signature::IsIdenticalParameters(const Signature& other) const {
  if (this == &other)
    return true;
  if (parameters_.size() != other.parameters_.size())
    return false;
  auto other_parameters = other.parameters_.begin();
  for (auto const parameter : parameters_) {
    if (!parameter->IsIdentical(**other_parameters))
      return false;
    ++other_parameters;
  }
  return true;
}

// Type
Type::Type() {
}

}  // namespace ir
}  // namespace compiler
}  // namespace elang
