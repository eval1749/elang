// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/visitor.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Visitor
//
Visitor::Visitor() {
}

Visitor::~Visitor() {
}

// Default implementations do nothing. Each derived visitor class implements
// override for interested classes.
#define V(type) \
  void Visitor::Visit##type(type* node) { DCHECK(node); }
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

}  // namespace ast
}  // namespace compiler
}  // namespace elang
