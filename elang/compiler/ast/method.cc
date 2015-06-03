// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/method.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

// Method
Method::Method(Zone* zone,
               Class* outer,
               Modifiers modifiers,
               Type* return_type,
               Token* name,
               const std::vector<Token*>& type_parameters)
    : ContainerNode(zone, outer, name, name),
      WithModifiers(modifiers),
      body_(nullptr),
      parameters_(zone),
      return_type_(return_type),
      type_parameters_(zone, type_parameters) {
  DCHECK(name->is_name());
  DCHECK_EQ(modifiers, Modifiers::Method() & modifiers);
}

ast::Class* Method::owner() const {
  return parent()->as<ast::Class>();
}

#if _DEBUG
// Node
bool Method::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>();
}
#endif

void Method::SetParameters(const std::vector<Parameter*>& parameters) {
  DCHECK(parameters_.empty());
  parameters_.reserve(parameters.size());
  parameters_.resize(0);
  for (auto const parameter : parameters)
    parameters_.push_back(parameter);
}

void Method::SetBody(Statement* body) {
  DCHECK(!body_);
  body_ = body;
}

// Parameter
Parameter::Parameter(Method* owner,
                     ParameterKind kind,
                     int position,
                     Type* type,
                     Token* name,
                     Expression* expression)
    : NamedNode(owner, name, name),
      kind_(kind),
      position_(position),
      type_(type),
      value_(expression) {
}

Method* Parameter::owner() const {
  return parent()->as<ast::Method>();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
