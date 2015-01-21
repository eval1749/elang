// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ir/factory.h"

#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/ir/visitor.h"

namespace elang {
namespace compiler {
namespace ir {

// Implementation of "visitor" pattern.
#define V(Name) \
  void Name::Accept(Visitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_CONCRETE_IR_NODE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory() {
}

Factory::~Factory() {
}

Class* Factory::NewClass(ast::Class* ast_class,
                         const std::vector<Class*>& base_classes) {
  return new (zone()) Class(zone(), ast_class, base_classes);
}

Literal* Factory::NewLiteral(Type* type, Token* token) {
  return new (zone()) Literal(type, token);
}

Method* Factory::NewMethod(ast::Method* ast_method, Signature* signature) {
  return new (zone()) Method(ast_method, signature);
}

Parameter* Factory::NewParameter(ast::Parameter* ast_parameter,
                                 Type* type,
                                 Value* default_value) {
  return new (zone()) Parameter(ast_parameter, type, default_value);
}

Signature* Factory::NewSignature(Type* return_type,
                                 const std::vector<Parameter*>& parameters) {
  return new (zone()) Signature(zone(), return_type, parameters);
}

Variable* Factory::NewVariable(Type* type,
                               StorageClass storage,
                               ast::NamedNode* ast_node) {
  return new (zone()) Variable(type, storage, ast_node);
}

}  // namespace ir
}  // namespace compiler
}  // namespace elang
