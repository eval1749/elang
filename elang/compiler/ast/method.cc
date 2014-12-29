// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/method.h"

#include "base/logging.h"
#include "elang/compiler/ast/method_group.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Method
//
Method::Method(NamespaceBody* namespace_body,
               MethodGroup* method_group,
               Modifiers modifiers,
               Expression* return_type,
               Token* name,
               const std::vector<Token*>& type_parameters,
               const std::vector<LocalVariable*>& parameters)
    : NamedNode(name, name),
      method_group_(method_group),
      modifiers_(modifiers),
      namespace_body_(namespace_body),
      parameters_(parameters),
      return_type_(return_type),
      statement_(nullptr),
      type_parameters_(type_parameters) {
  DCHECK(name->is_name());
  DCHECK_EQ(method_group_->name()->simple_name(), name->simple_name());
}

Method::~Method() {
}

void Method::SetStatement(ast::Statement* statement) {
  DCHECK(!statement_);
  DCHECK(statement);
  statement_ = statement;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
