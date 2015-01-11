// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/method.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
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
               NamespaceBody* namespace_body,
               MethodGroup* method_group,
               Modifiers modifiers,
               Expression* return_type,
               Token* name,
               const std::vector<Token*>& type_parameters,
               const std::vector<LocalVariable*>& parameters)
    : NamespaceMember(namespace_body, modifiers, name, name),
      body_(nullptr),
      method_group_(method_group),
      parameters_(zone, parameters),
      return_type_(return_type),
      type_parameters_(zone, type_parameters) {
  DCHECK(name->is_name());
  DCHECK_EQ(method_group_->name()->simple_name(), name->simple_name());
}

void Method::SetBody(ast::Statement* statement) {
  DCHECK(!body_);
  DCHECK(statement);
  body_ = statement;
}

//////////////////////////////////////////////////////////////////////
//
// MethodGroup
//
MethodGroup::MethodGroup(Zone* zone, NamespaceBody* namespace_body, Token* name)
    : NamespaceMember(namespace_body, Modifiers(), name, name), methods_(zone) {
  DCHECK(name->is_name());
  DCHECK(namespace_body->owner()->is<Class>());
}

void MethodGroup::AddMethod(Method* method) {
  DCHECK_EQ(method->method_group(), this);
  DCHECK(std::find(methods_.begin(), methods_.end(), method) == methods_.end());
  methods_.push_back(method);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
