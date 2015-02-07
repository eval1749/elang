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

  // Returns a list of graph node.
  const Nodes& nodes() const { return nodes_; }

  OrderedList<Derived*> ComputePreOrderList() const;
  OrderedList<Derived*> ComputePostOrderList() const;
  OrderedList<Derived*> ComputeReversePreOrderList() const;
  OrderedList<Derived*> ComputeReversePostOrderList() const;
  bool HasEdge(Derived* from, Derived* to) const;

 protected:
  Graph() = default;
  ~Graph() = default;

 private:
  friend class Editor;

  enum class Reverse {
    No,
    Yes,
  };

  enum class Order {
    PreOrder,
    PostOrder,
  };

  class Sorter {
   public:
    Sorter(const Graph* graph, Order order, Reverse reverse);
    ~Sorter() = default;

    OrderedList<Derived*> Build();

   private:
    void Visit(typename OrderedList<Derived*>::Builder* builder, Derived* node);

    const Graph* const graph_;
    Order const order_;
    Reverse const reverse_;
    std::unordered_set<Derived*> visited_;

    DISALLOW_COPY_AND_ASSIGN(Sorter);
  };

  Nodes nodes_;

  DISALLOW_COPY_AND_ASSIGN(Graph);
};

// Graph
template <typename Owner, typename T>
OrderedList<T*> Graph<Owner, T>::ComputePreOrderList() const {
  Sorter builder(this, Order::PreOrder, Reverse::No);
  return builder.Build();
}

template <typename Owner, typename T>
OrderedList<T*> Graph<Owner, T>::ComputePostOrderList() const {
  Sorter builder(this, Order::PostOrder, Reverse::No);
  return builder.Build();
}

template <typename Owner, typename T>
OrderedList<T*> Graph<Owner, T>::ComputeReversePreOrderList() const {
  Sorter builder(this, Order::PreOrder, Reverse::Yes);
  return builder.Build();
}

template <typename Owner, typename T>
OrderedList<T*> Graph<Owner, T>::ComputeReversePostOrderList() const {
  Sorter builder(this, Order::PostOrder, Reverse::Yes);
  return builder.Build();
}

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

// Graph::Sorter
template <typename Owner, typename T>
Graph<Owner, T>::Sorter::Sorter(const Graph* graph,
                                Order order,
                                Reverse reverse)
    : graph_(graph), order_(order), reverse_(reverse) {
}

template <typename Owner, typename Derived>
OrderedList<Derived*> Graph<Owner, Derived>::Sorter::Build() {
  OrderedList<Derived*>::Builder builder;
  Visit(&builder, *graph_->nodes().begin());
  if (reverse_ == Reverse::Yes)
    builder.Reverse();
  return builder.Get();
}

template <typename Owner, typename Derived>
void Graph<Owner, Derived>::Sorter::Visit(
    typename OrderedList<Derived*>::Builder* builder,
    Derived* node) {
  if (visited_.count(node))
    return;
  visited_.insert(node);
  if (order_ == Order::PreOrder)
    builder->Add(node);
  for (auto const successor : node->successors())
    Visit(builder, successor);
  if (order_ == Order::PostOrder)
    builder->Add(node);
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPHS_GRAPH_H_
