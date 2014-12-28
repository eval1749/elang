// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/field.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Field
//
Field::Field(NamespaceBody* namespace_body,
             Modifiers modifiers,
             Expression* type,
             Token* name,
             Expression* expression)
    : NamespaceMember(namespace_body, modifiers, name, name),
      expression_(expression),
      type_(type) {
  DCHECK(namespace_body->owner()->is<Class>());
}

Field::~Field() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
