// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPHS_FLOW_GRAPH_H_
#define ELANG_BASE_GRAPHS_FLOW_GRAPH_H_

namespace elang {

// BackwardFlowGraph is used for computing post-dominator tree
template <typename Graph>
struct BackwardFlowGraph {
  typedef typename Graph::Derived GraphNode;

  static GraphNode* EntryOf(const Graph* graph) { return graph->last_node(); }

  static bool HasMoreThanOnePredecessors(const GraphNode* node) {
    return node->HasMoreThanOneSuccessors();
  }

  static bool HasMoreThanOneSuccessor(const GraphNode* node) {
    return node->HasMoreThanOnePredecessors();
  }

  static typename Graph::NodeSet PredecessorsOf(const GraphNode* node) {
    return node->successors();
  }

  static typename Graph::NodeSet SuccessorsOf(const GraphNode* node) {
    return node->predecessors();
  }
};

// ForwardFlowGraph is used for computing dominator tree
template <typename Graph>
struct ForwardFlowGraph {
  typedef typename Graph::Derived GraphNode;

  static GraphNode* EntryOf(const Graph* graph) { return graph->first_node(); }

  static bool HasMoreThanOnePredecessors(const GraphNode* node) {
    return node->HasMoreThanOnePredecessors();
  }

  static bool HasMoreThanOneSuccessors(const GraphNode* node) {
    return node->HasMoreThanOneSuccessor();
  }

  static typename Graph::NodeSet PredecessorsOf(const GraphNode* node) {
    return node->predecessors();
  }

  static typename Graph::NodeSet SuccessorsOf(const GraphNode* node) {
    return node->successors();
  }
};

}  // namespace elang

#endif  // ELANG_BASE_GRAPHS_FLOW_GRAPH_H_
