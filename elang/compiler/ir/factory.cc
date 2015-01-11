// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ir/factory.h"

#include "elang/compiler/ir/nodes.h"

namespace elang {
namespace compiler {
namespace ir {

Factory::Factory() {
}

Factory::~Factory() {
}

Class* Factory::NewClass(ast::Class* ast_class,
                         const std::vector<Class*>& base_classes) {
  return new (zone()) Class(zone(), ast_class, base_classes);
}

Parameter* Factory::NewParameter(ParameterKind kind,
                                 Token* name,
                                 Type* type,
                                 Value* default_value) {
  return new (zone()) Parameter(kind, name, type, default_value);
}

Method* Factory::NewMethod(ast::Method* ast_method, Signature* signature) {
  return new (zone()) Method(ast_method, signature);
}

Signature* Factory::NewSignature(Type* return_type,
                                 const std::vector<Parameter*>& parameters) {
  return new (zone()) Signature(zone(), return_type, parameters);
}

}  // namespace ir
}  // namespace compiler
}  // namespace elang
