// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/hir/analysis/graph.h"

#include "base/logging.h"
#include "elang/base/zone_user.h"
#include "elang/hir/analysis/dominator_tree.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ListBuilder
//
enum Order {
  PreOrder,
  PostOrder,
  ReversePostOrder,
  ReversePreOrder,
};

class ListBuilder final {
 public:
  typedef OrderedList<Value*> List;

  ListBuilder(const Graph* graph, Order order);
  ~ListBuilder() = default;

  List Build();

 private:
  void Visit(Value* value, List::Builder* builder);

  const Graph* const graph_;
  Order const order_;
  std::unordered_set<Value*> visited_;

  DISALLOW_COPY_AND_ASSIGN(ListBuilder);
};

ListBuilder::ListBuilder(const Graph* graph, Order order)
    : graph_(graph), order_(order) {
}

OrderedList<Value*> ListBuilder::Build() {
  List::Builder list_builder;
  Visit(graph_->entry(), &list_builder);
  if (order_ == Order::ReversePostOrder || order_ == Order::ReversePreOrder)
    list_builder.Reverse();
  return list_builder.Get();
}

void ListBuilder::Visit(Value* value, List::Builder* list_builder) {
  if (visited_.count(value))
    return;
  visited_.insert(value);
  if (order_ == Order::PreOrder || order_ == Order::ReversePreOrder)
    list_builder->Add(value);
  for (auto const successor : graph_->SuccessorsOf(value))
    Visit(successor, list_builder);
  if (order_ == Order::PostOrder || order_ == Order::ReversePostOrder)
    list_builder->Add(value);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Graph::Provider
//
Graph::Provider::Provider() {
}

Graph::Provider::~Provider() {
}

//////////////////////////////////////////////////////////////////////
//
// Graph
//
Graph::Graph(Provider* provider) : provider_(provider) {
}

Graph::~Graph() {
}

Value* Graph::entry() const {
  return provider_->entry();
}

std::vector<Value*> Graph::PredecessorsOf(Value* value) const {
  return std::vector<Value*>(provider_->PredecessorsOf(value));
}

std::vector<Value*> Graph::SuccessorsOf(Value* value) const {
  return std::vector<Value*>(provider_->SuccessorsOf(value));
}

bool Graph::HasMoreThanOnePredecessors(Value* value) const {
  return provider_->HasMoreThanOnePredecessors(value);
}

OrderedList<Value*> Graph::PostOrderList() const {
  return ListBuilder(this, Order::PostOrder).Build();
}

OrderedList<Value*> Graph::PreOrderList() const {
  return ListBuilder(this, Order::PreOrder).Build();
}

OrderedList<Value*> Graph::ReversePostOrderList() const {
  return ListBuilder(this, Order::ReversePostOrder).Build();
}

OrderedList<Value*> Graph::ReversePreOrderList() const {
  return ListBuilder(this, Order::ReversePreOrder).Build();
}

//////////////////////////////////////////////////////////////////////
//
// ControlFlowGraph
//
ControlFlowGraph::ControlFlowGraph(Function* function)
    : Graph(this), function_(function) {
}

// Graph::Provider
Value* ControlFlowGraph::entry() const {
  return function_->entry_block();
}

bool ControlFlowGraph::HasMoreThanOnePredecessors(Value* value) const {
  auto count = 0;
  for (auto predecessor : value->as<BasicBlock>()->predecessors()) {
    DCHECK(predecessor);
    ++count;
    if (count == 2)
      return true;
  }
  return false;
}

std::vector<Value*> ControlFlowGraph::PredecessorsOf(Value* value) const {
  std::vector<Value*> predecessors;
  for (auto predecessor : value->as<BasicBlock>()->predecessors())
    predecessors.push_back(predecessor);
  return predecessors;
}

std::vector<Value*> ControlFlowGraph::SuccessorsOf(Value* value) const {
  std::vector<Value*> successors;
  for (auto successor : value->as<BasicBlock>()->successors())
    successors.push_back(successor);
  return successors;
}

}  // namespace hir
}  // namespace elang
