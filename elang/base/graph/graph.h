// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPH_GRAPH_H_
#define ELANG_BASE_GRAPH_GRAPH_H_

#include "elang/base/double_linked.h"
#include "elang/base/zone_unordered_set.h"

namespace elang {

template <typename Owner, typename Derived>
class GraphEditor;

//////////////////////////////////////////////////////////////////////
//
// Represents directed graph. It is OK to have cycle.
//
template <typename Owner, typename Derived>
class Graph {
 public:
  typedef GraphEditor<Owner, Derived> Editor;
  typedef DoubleLinked<Derived, Owner> Nodes;

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
    bool HasSucccessor() const { return !successors_.empty(); }

   protected:
    explicit Node(Zone* zone);
    ~Node() = default;

   private:
    friend class Editor;

    // For control flow graph, most of most of basic block has only one
    // successors, e.g. by unconditional jump, and return. More than two
    // successors are introduced by switch like statement. So, using
    // |ZoneUnorderedSet| may be overkill regarding memory consumption.
    ZoneUnorderedSet<Derived*> predecessors_;
    ZoneUnorderedSet<Derived*> successors_;

    DISALLOW_COPY_AND_ASSIGN(Node);
  };

  // Returns a list of graph node.
  const Nodes& nodes() const { return nodes_; }

 protected:
  Graph() = default;
  ~Graph() = default;

 private:
  friend class Editor;

  Nodes nodes_;

  DISALLOW_COPY_AND_ASSIGN(Graph);
};

template <typename Owner, typename T>
Graph<Owner, T>::Node::Node(Zone* zone)
    : predecessors_(zone), successors_(zone) {
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPH_GRAPH_H_
