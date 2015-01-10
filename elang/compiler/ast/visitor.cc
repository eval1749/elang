// Copyright 2014 Project Vogue. All rights reserved.
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


#define DEF_VISIT(type) \
  void Visitor::Visit##type(type* node) { DCHECK(node); }
  FOR_EACH_AST_NODE(DEF_VISIT)
#undef DEF_VISIT

}  // namespace ast
}  // namespace compiler
}  // namespace elang
