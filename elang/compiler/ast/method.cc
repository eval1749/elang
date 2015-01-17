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

//////////////////////////////////////////////////////////////////////
//
// Method
//
Method::Method(Zone* zone,
               ClassBody* outer,
               MethodGroup* method_group,
               Modifiers modifiers,
               Type* return_type,
               Token* name,
               const std::vector<Token*>& type_parameters)
    : NamespaceNode(zone, outer, name, name),
      WithModifiers(modifiers),
      body_(nullptr),
      method_group_(method_group),
      parameters_(zone),
      return_type_(return_type),
      type_parameters_(zone, type_parameters) {
  DCHECK(name->is_name());
  DCHECK_EQ(method_group_->name()->atomic_string(), name->atomic_string());
  DCHECK_EQ(modifiers, Modifiers::Method() & modifiers);
}

ast::Class* Method::owner() const {
  return parent()->as<ast::ClassBody>()->owner();
}

#if _DEBUG
// Node
bool Method::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::ClassBody>();
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

//////////////////////////////////////////////////////////////////////
//
// MethodGroup
//
MethodGroup::MethodGroup(Zone* zone, Class* owner, Token* name)
    : NamedNode(owner, name, name), methods_(zone) {
  DCHECK(name->is_name());
}

Class* MethodGroup::owner() const {
  return parent()->as<ast::Class>();
}

void MethodGroup::AddMethod(Method* method) {
  DCHECK_EQ(method->method_group(), this);
  DCHECK(std::find(methods_.begin(), methods_.end(), method) == methods_.end());
  methods_.push_back(method);
}

#if _DEBUG
// NamedNode
bool MethodGroup::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>() || container->is<ast::ClassBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Parameter
//
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
