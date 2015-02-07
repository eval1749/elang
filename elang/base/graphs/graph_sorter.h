// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPHS_GRAPH_SORTER_H_
#define ELANG_BASE_GRAPHS_GRAPH_SORTER_H_

#include "elang/base/graphs/graph.h"
#include "elang/base/ordered_list.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// Sort graph.
//
template <typename Owner, typename Derived>
class GraphSorter final {
 public:
  typedef Graph<Owner, Derived> Graph;
  typedef OrderedList<Derived*> OrderedList;

  static OrderedList SortByPreOrder(const Graph* graph);
  static OrderedList SortByPostOrder(const Graph* graph);
  static OrderedList SortByReversePreOrder(const Graph* graph);
  static OrderedList SortByReversePostOrder(const Graph* graph);

 private:
  enum class Reverse {
    No,
    Yes,
  };

  enum class Order {
    PreOrder,
    PostOrder,
  };

  GraphSorter(const Graph* graph, Order order, Reverse reverse);
  ~GraphSorter() = default;

  OrderedList Sort();
  void Visit(typename OrderedList::Builder* builder, Derived* node);

  const Graph* const graph_;
  Order const order_;
  Reverse const reverse_;
  std::unordered_set<Derived*> visited_;

  DISALLOW_COPY_AND_ASSIGN(GraphSorter);
};

// GraphSorter

// Graph::GraphSorter
template <typename Owner, typename Derived>
GraphSorter<Owner, Derived>::GraphSorter(const Graph* graph,
                                         Order order,
                                         Reverse reverse)
    : graph_(graph), order_(order), reverse_(reverse) {
}

template <typename Owner, typename Derived>
OrderedList<Derived*> GraphSorter<Owner, Derived>::Sort() {
  OrderedList::Builder builder;
  Visit(&builder, *graph_->nodes().begin());
  if (reverse_ == Reverse::Yes)
    builder.Reverse();
  return builder.Get();
}

template <typename Owner, typename Derived>
void GraphSorter<Owner, Derived>::Visit(typename OrderedList::Builder* builder,
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

template <typename Owner, typename T>
OrderedList<T*> GraphSorter<Owner, T>::SortByPreOrder(const Graph* graph) {
  return GraphSorter(graph, Order::PreOrder, Reverse::No).Sort();
}

template <typename Owner, typename T>
OrderedList<T*> GraphSorter<Owner, T>::SortByPostOrder(const Graph* graph) {
  return GraphSorter(graph, Order::PostOrder, Reverse::No).Sort();
}

template <typename Owner, typename T>
OrderedList<T*> GraphSorter<Owner, T>::SortByReversePreOrder(
    const Graph* graph) {
  return GraphSorter(graph, Order::PreOrder, Reverse::Yes).Sort();
}

template <typename Owner, typename T>
OrderedList<T*> GraphSorter<Owner, T>::SortByReversePostOrder(
    const Graph* graph) {
  return GraphSorter(graph, Order::PostOrder, Reverse::Yes).Sort();
}

}  // namespace elang

#endif  // ELANG_BASE_GRAPHS_GRAPH_SORTER_H_
