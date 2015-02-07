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

template <typename Owner, typename Derived>
class GraphEditor;

template <typename Owner, typename Derived>
class GraphSorter;

//////////////////////////////////////////////////////////////////////
//
// Represents directed graph. It is OK to have cycle.
//
template <typename Owner, typename Derived>
class Graph {
 public:
  typedef GraphEditor<Owner, Derived> Editor;
  typedef Derived Derived;
  typedef DoubleLinked<Derived, Owner> Nodes;
  typedef GraphSorter<Owner, Derived> Sorter;

  // Node represents graph node having edges.
  class Node : public DoubleLinked<Derived, Owner>::Node {
   public:
    const ZoneUnorderedSet<Derived*>& predecessors() const {
      return predecessors_;
    }
    const ZoneUnorderedSet<Derived*>& successors() const { return successors_; }

    bool HasMoreThanOnePredecessors() const {
      return predecessors_.size() > 1u;
    }
    bool HasPredecessor() const { return !predecessors_.empty(); }
    bool HasSuccessor() const { return !successors_.empty(); }

   protected:
    explicit Node(Zone* zone);
    ~Node() = default;

   private:
    friend class Editor;
    friend class Graph;

    // For control flow graph, most of most of basic block has only one
    // successors, e.g. by unconditional jump, and return. More than two
    // successors are introduced by switch like statement. So, using
    // |ZoneUnorderedSet| may be overkill regarding memory consumption.
    ZoneUnorderedSet<Derived*> predecessors_;
    ZoneUnorderedSet<Derived*> successors_;

    DISALLOW_COPY_AND_ASSIGN(Node);
  };

  // Returns first block.
  Derived* first_node() const { return nodes_.first_node(); }
  Derived* last_node() const { return nodes_.last_node(); }

  // Returns a list of graph node.
  const Nodes& nodes() const { return nodes_; }

  bool HasEdge(Derived* from, Derived* to) const;

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

// Graph::Node
template <typename Owner, typename T>
Graph<Owner, T>::Node::Node(Zone* zone)
    : predecessors_(zone), successors_(zone) {
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPHS_GRAPH_H_
