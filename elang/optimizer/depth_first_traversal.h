// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_DEPTH_FIRST_TRAVERSAL_H_
#define ELANG_OPTIMIZER_DEPTH_FIRST_TRAVERSAL_H_

#include <stack>
#include <vector>

#include "base/macros.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_visitor.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// DepthFirstTraversal
//
template <typename Strategy, typename Context>
class DepthFirstTraversal {
 public:
  DepthFirstTraversal() {}
  ~DepthFirstTraversal() {}

  void Traverse(Context* context, NodeVisitor* visitor);

 private:
  DISALLOW_COPY_AND_ASSIGN(DepthFirstTraversal);
};

template <typename Strategy, typename Context>
void DepthFirstTraversal<Strategy, Context>::Traverse(Context* context,
                                                      NodeVisitor* visitor) {
  enum class State {
    NotVisited,
    OnStack,
    Visited,
  };
  std::vector<State> states(context->max_node_id() + 1, State::NotVisited);
  std::stack<Node*> stack;
  stack.push(Strategy::StartNodeOf(context));
  states[stack.top()->id()] = State::OnStack;
  while (!stack.empty()) {
    auto const node = stack.top();
    auto should_pop = true;
    for (auto successor : Strategy::SuccessorsOf(context, node)) {
      if (!Strategy::ShouldVisit(context, node))
        continue;
      if (states[successor->id()] != State::NotVisited)
        continue;
      stack.push(successor);
      states[successor->id()] = State::OnStack;
      should_pop = false;
      break;
    }
    if (!should_pop)
      continue;
    states[node->id()] = State::Visited;
    stack.pop();
    node->Accept(visitor);
  }
}

//////////////////////////////////////////////////////////////////////
//
// OnInputEdge
//
struct OnInputEdge {
  static bool ShouldVisit(const Function* function, Node* node);
  static Node* StartNodeOf(const Function* function);
  static Node::Inputs SuccessorsOf(const Function* function, Node* node);
};

//////////////////////////////////////////////////////////////////////
//
// OnControlEdge
//
struct OnControlEdge : OnInputEdge {
  static bool ShouldVisit(const Function* function, Node* node);
};

//////////////////////////////////////////////////////////////////////
//
// OnEffectEdge
//
struct OnEffectEdge : OnInputEdge {
  static bool ShouldVisit(const Function* function, Node* node);
};

//////////////////////////////////////////////////////////////////////
//
// OnUseEdge
//
struct OnUseEdge {
  static bool ShouldVisit(const Function* function, Node* node);
  static Node* StartNodeOf(const Function* function);
  static const Node::Users& SuccessorsOf(const Function* function, Node* node);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_DEPTH_FIRST_TRAVERSAL_H_
