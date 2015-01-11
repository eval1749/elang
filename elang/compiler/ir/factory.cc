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

Parameter* Factory::NewParameter(Token* name,
                                        Type* type,
                                        Value* default_value) {
  return new (zone()) Parameter(name, type, default_value);
}

Signature* Factory::NewSignature(
    Token* name,
    Type* return_type,
    const std::vector<Parameter*>& parameters) {
  return new (zone()) Signature(zone(), name, return_type, parameters);
}

}  // namespace ir
}  // namespace compiler
}  // namespace elang
