// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPHS_GRAPH_H_
#define ELANG_BASE_GRAPHS_GRAPH_H_

#include "base/logging.h"
#include "elang/base/double_linked.h"
#include "elang/base/ordered_list.h"
#include "elang/base/zone_unordered_set.h"

namespace elang {

template <typename Graph>
struct ForwardFlowGraph;

template <typename Graph>
class GraphEditor;

template <typename Graph, typename Direction>
class GraphSorter;

//////////////////////////////////////////////////////////////////////
//
// Represents directed graph. It is OK to have cycle.
//
template <typename Owner, typename Node>
class Graph {
 public:
  using GraphNode = Node;
  using Editor = GraphEditor<Owner>;
  using NodeSet = ZoneUnorderedSet<GraphNode*>;
  using Nodes = DoubleLinked<GraphNode, Owner>;
  using Sorter = GraphSorter<Graph, ForwardFlowGraph<Graph>>;

  // |NodeBase| represents graph node having edges.
  class GraphNodeBase : public DoubleLinked<GraphNode, Owner>::Node {
   public:
    const NodeSet& predecessors() const { return predecessors_; }
    const NodeSet& successors() const { return successors_; }

    bool HasMoreThanOnePredecessor() const { return predecessors_.size() > 1u; }
    bool HasMoreThanOneSuccessor() const { return successors_.size() > 1u; }
    bool HasPredecessor() const { return !predecessors_.empty(); }
    bool HasSuccessor() const { return !successors_.empty(); }

   protected:
    explicit GraphNodeBase(Zone* zone);
    ~GraphNodeBase() = default;

   private:
    friend class Editor;
    friend class Graph;

    // For control flow graph, most of most of basic block has only one
    // successors, e.g. by unconditional jump, and return. More than two
    // successors are introduced by switch like statement. So, using
    // |ZoneUnorderedSet| may be overkill regarding memory consumption.
    NodeSet predecessors_;
    NodeSet successors_;

    DISALLOW_COPY_AND_ASSIGN(GraphNodeBase);
  };

  // Returns first block.
  GraphNode* first_node() const { return nodes_.first_node(); }
  GraphNode* last_node() const { return nodes_.last_node(); }

  // Returns a list of graph node.
  const Nodes& nodes() const { return nodes_; }

  bool HasEdge(GraphNode* from, GraphNode* to) const;

 protected:
  Graph() = default;
  ~Graph() = default;

 private:
  friend class Editor;

  Nodes nodes_;

  DISALLOW_COPY_AND_ASSIGN(Graph);
};

// Graph
template <typename Owner, typename T>
bool Graph<Owner, T>::HasEdge(T* from, T* to) const {
  if (from->successors_.count(to) == 1) {
    DCHECK_EQ(to->predecessors_.count(from), 1u);
    return true;
  }
  DCHECK_EQ(to->predecessors_.count(from), 0u);
  return false;
}

// Graph::GraphNodeBase
template <typename Owner, typename T>
Graph<Owner, T>::GraphNodeBase::GraphNodeBase(Zone* zone)
    : predecessors_(zone), successors_(zone) {
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPHS_GRAPH_H_
