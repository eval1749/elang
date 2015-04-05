// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/visitor.h"

#include "base/logging.h"
#include "elang/compiler/ast/nodes.h"

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

// We implement |DoDefaultVisit()| and "VisitorXXX()" in "factory.cc" to avoid
// include AST node include files in this file for improve compilation speed.

void Visitor::Visit(const Node* node) {
  const_cast<Node*>(node)->Accept(this);
}

void Visitor::Visit(Node* node) {
  node->Accept(this);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
