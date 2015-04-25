// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_DEPTH_FIRST_TRAVERSAL_H_
#define ELANG_OPTIMIZER_DEPTH_FIRST_TRAVERSAL_H_

#include <stack>
#include <vector>

#include "base/macros.h"
#include "elang/optimizer/function.h"
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
    auto const from = stack.top();
    auto should_pop = true;
    for (auto edge : Strategy::AdjacentEdgesOf(context, from)) {
      auto const to = Strategy::EdgeTo(edge);
      if (!Strategy::ShouldVisit(context, to))
        continue;
      if (states[to->id()] != State::NotVisited)
        continue;
      stack.push(to);
      states[to->id()] = State::OnStack;
      should_pop = false;
      break;
    }
    if (!should_pop)
      continue;
    states[from->id()] = State::Visited;
    stack.pop();
    from->Accept(visitor);
  }
}

//////////////////////////////////////////////////////////////////////
//
// OnInputEdge
//
struct OnInputEdge {
  static Node::Inputs AdjacentEdgesOf(const Function* function, Node* node);
  static Node* EdgeTo(Node* node);
  static bool ShouldVisit(const Function* function, Node* node);
  static Node* StartNodeOf(const Function* function);
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
  static const UseEdges& AdjacentEdgesOf(const Function* function, Node* node);
  static Node* EdgeTo(const UseEdge* edge);
  static bool ShouldVisit(const Function* function, Node* node);
  static Node* StartNodeOf(const Function* function);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_DEPTH_FIRST_TRAVERSAL_H_
