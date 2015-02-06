// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPH_GRAPH_EDITOR_H_
#define ELANG_BASE_GRAPH_GRAPH_EDITOR_H_

#include <vector>

#include "base/logging.h"
#include "elang/base/graph/graph.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// Editor edits a Graph.
//
template <typename Owner, typename Derived>
class GraphEditor {
 public:
  typedef Graph<Owner, Derived> Graph;

  explicit GraphEditor(Graph* graph) : graph_(graph) {}
  ~GraphEditor() = default;

  void AppendNode(Derived* new_node);
  void AddEdge(Derived* from, Derived* to);
  void InsertNode(Derived* new_node, Derived* ref_node);
  void RemoveEdge(Derived* from, Derived* to);
  void RemoveNode(Derived* ref_node);

 private:
  Graph* const graph_;

  DISALLOW_COPY_AND_ASSIGN(GraphEditor);
};

template <typename Owner, typename T>
void GraphEditor<Owner, T>::AppendNode(T* new_node) {
  graph_->nodes_.AppendNode(new_node);
}

template <typename Owner, typename T>
void GraphEditor<Owner, T>::AddEdge(T* from, T* to) {
  from->successors_.insert(to);
  to->predecessors_.insert(from);
}

template <typename Owner, typename T>
void GraphEditor<Owner, T>::InsertNode(T* new_node, T* ref_node) {
  graph_->nodes_.InsertBefore(new_node, ref_node);
}

template <typename Owner, typename T>
void GraphEditor<Owner, T>::RemoveEdge(T* from, T* to) {
  DCHECK(from->successors_.count(to));
  DCHECK(to->predecessors_.count(from));
  from->successors_.erase(to);
  to->predecessors_.erase(from);
}

template <typename Owner, typename T>
void GraphEditor<Owner, T>::RemoveNode(T* old_node) {
  graph_->nodes_.RemoveNode(old_node);
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPH_GRAPH_EDITOR_H_
