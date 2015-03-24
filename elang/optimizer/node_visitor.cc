// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/node_visitor.h"

#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// NodeVisitor
//
NodeVisitor::NodeVisitor() {
}

NodeVisitor::~NodeVisitor() {
}

void NodeVisitor::DoDefaultVisit(Node* node) {
}

#define V(Name, ...) \
  void NodeVisitor::Visit##Name(Name##Node* node) { DoDefaultVisit(node); }
FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V

}  // namespace optimizer
}  // namespace elang
