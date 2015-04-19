// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPHS_GRAPH_EDITOR_H_
#define ELANG_BASE_GRAPHS_GRAPH_EDITOR_H_

#include <vector>

#include "base/logging.h"
#include "elang/base/graphs/graph.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// Editor edits a Graph.
//
template <typename Graph>
class GraphEditor {
 public:
  using GraphNode = typename Graph::GraphNode;

  explicit GraphEditor(Graph* graph) : graph_(graph) {}
  ~GraphEditor() = default;

  void AppendNode(GraphNode* new_node);
  void AddEdge(GraphNode* from, GraphNode* to);
  void InsertNode(GraphNode* new_node, GraphNode* ref_node);
  void RemoveEdge(GraphNode* from, GraphNode* to);
  void RemoveNode(GraphNode* ref_node);

 private:
  Graph* const graph_;

  DISALLOW_COPY_AND_ASSIGN(GraphEditor);
};

template <typename Graph>
void GraphEditor<Graph>::AppendNode(typename Graph::GraphNode* new_node) {
  graph_->nodes_.AppendNode(new_node);
}

template <typename Graph>
void GraphEditor<Graph>::AddEdge(typename Graph::GraphNode* from,
                                 typename Graph::GraphNode* to) {
  from->successors_.insert(to);
  to->predecessors_.insert(from);
}

template <typename Graph>
void GraphEditor<Graph>::InsertNode(typename Graph::GraphNode* new_node,
                                    typename Graph::GraphNode* ref_node) {
  graph_->nodes_.InsertBefore(new_node, ref_node);
}

template <typename Graph>
void GraphEditor<Graph>::RemoveEdge(typename Graph::GraphNode* from,
                                    typename Graph::GraphNode* to) {
  from->successors_.erase(to);
  to->predecessors_.erase(from);
}

template <typename Graph>
void GraphEditor<Graph>::RemoveNode(typename Graph::GraphNode* old_node) {
  graph_->nodes_.RemoveNode(old_node);
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPHS_GRAPH_EDITOR_H_
