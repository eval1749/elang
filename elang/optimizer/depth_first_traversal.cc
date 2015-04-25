// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/depth_first_traversal.h"

#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// OnControlEdge
//
bool OnControlEdge::ShouldVisit(const Function* function, Node* node) {
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// OnEffectEdge
//
bool OnEffectEdge::ShouldVisit(const Function* function, Node* node) {
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// OnInputEdge
//
Node::Inputs OnInputEdge::AdjacentEdgesOf(const Function* function,
                                          Node* node) {
  return node->inputs();
}

Node* OnInputEdge::EdgeTo(Node* node) {
  return node;
}

bool OnInputEdge::ShouldVisit(const Function* function, Node* node) {
  return true;
}

Node* OnInputEdge::StartNodeOf(const Function* function) {
  return function->exit_node();
}

//////////////////////////////////////////////////////////////////////
//
// OnUseEdge
//
const UseEdges& OnUseEdge::AdjacentEdgesOf(const Function* function,
                                           Node* node) {
  return node->use_edges();
}

Node* OnUseEdge::EdgeTo(const UseEdge* edge) {
  return edge->from();  // returns user
}

bool OnUseEdge::ShouldVisit(const Function* function, Node* node) {
  return true;
}

Node* OnUseEdge::StartNodeOf(const Function* function) {
  return function->entry_node();
}

}  // namespace optimizer
}  // namespace elang
