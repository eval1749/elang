// Copyright 2014 Project Vogue. All rights reserved.
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
               Class* owner,
               MethodGroup* method_group,
               Modifiers modifiers,
               Expression* return_type,
               Token* name,
               const std::vector<Token*>& type_parameters,
               const std::vector<LocalVariable*>& parameters)
    : ContainerNode(zone, owner, name, name),
      WithModifiers(modifiers),
      body_(nullptr),
      method_group_(method_group),
      parameters_(zone, parameters),
      return_type_(return_type),
      type_parameters_(zone, type_parameters) {
  DCHECK(name->is_name());
  DCHECK_EQ(method_group_->name()->atomic_string(), name->atomic_string());
}

ast::Class* Method::owner() const {
  return parent()->as<ast::Class>();
}

void Method::SetBody(ast::Statement* statement) {
  DCHECK(!body_);
  DCHECK(statement);
  body_ = statement;
}

#if _DEBUG
// Node
bool Method::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::Class>();
}
#endif

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
  return container->is<ast::Class>();
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
