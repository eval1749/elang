// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPHS_GRAPH_EDITOR_H_
#define ELANG_BASE_GRAPHS_GRAPH_EDITOR_H_

#include <algorithm>

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
  using Node = typename Graph::GraphNode;
  using NodeList = typename Graph::NodeList;

  explicit GraphEditor(Graph* graph) : graph_(graph) {}
  ~GraphEditor() = default;

  void AppendNode(Node* new_node);
  void AddEdge(Node* from, Node* to);
  void InsertNode(Node* new_node, Node* ref_node);
  void RemoveEdge(Node* from, Node* to);
  void RemoveNode(Node* ref_node);

 private:
  static void RemoveFromList(NodeList* nodes, Node* node);

  Graph* const graph_;

  DISALLOW_COPY_AND_ASSIGN(GraphEditor);
};

template <typename Graph>
void GraphEditor<Graph>::AppendNode(Node* new_node) {
  graph_->nodes_.AppendNode(new_node);
}

template <typename Graph>
void GraphEditor<Graph>::AddEdge(Node* from, Node* to) {
  DCHECK(!graph_->HasEdge(from, to));
  from->successors_.push_back(to);
  to->predecessors_.push_back(from);
}

template <typename Graph>
void GraphEditor<Graph>::InsertNode(Node* new_node, Node* ref_node) {
  graph_->nodes_.InsertBefore(new_node, ref_node);
}

template <typename Graph>
void GraphEditor<Graph>::RemoveEdge(Node* from, Node* to) {
  RemoveFromList(&from->successors_, to);
  RemoveFromList(&to->predecessors_, from);
}

template <typename Graph>
void GraphEditor<Graph>::RemoveFromList(NodeList* nodes, Node* node) {
  auto const it = std::find(nodes->begin(), nodes->end(), node);
  DCHECK(it != nodes->end());
  nodes->erase(it);
}

template <typename Graph>
void GraphEditor<Graph>::RemoveNode(Node* old_node) {
  graph_->nodes_.RemoveNode(old_node);
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPHS_GRAPH_EDITOR_H_
