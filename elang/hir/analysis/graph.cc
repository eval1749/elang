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

  ListBuilder(Graph* graph, Order order);
  ~ListBuilder() = default;

  List Build();

 private:
  void Visit(Value* value, List::Builder* builder);

  Graph* const graph_;
  Order const order_;
  std::unordered_set<Value*> visited_;

  DISALLOW_COPY_AND_ASSIGN(ListBuilder);
};

ListBuilder::ListBuilder(Graph* graph, Order order)
    : graph_(graph), order_(order) {
}

OrderedList<Value*> ListBuilder::Build() {
  List::Builder list_builder;
  Visit(graph_->entry(), &list_builder);
  return list_builder.Get();
}

void ListBuilder::Visit(Value* value, List::Builder* list_builder) {
  if (visited_.count(value))
    return;
  visited_.insert(value);
  for (auto const successor : graph_->successors_of(value))
    Visit(successor, list_builder);
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

std::vector<Value*> Graph::predecessors_of(Value* value) {
  return std::vector<Value*>(provider_->predecessors_of(value));
}

std::vector<Value*> Graph::successors_of(Value* value) {
  return std::vector<Value*>(provider_->successors_of(value));
}

bool Graph::HasMoreThanOnePredecessors(Value* value) {
  return provider_->HasMoreThanOnePredecessors(value);
}

OrderedList<Value*> Graph::ReversePostOrderList() {
  return ListBuilder(this, Order::ReversePostOrder).Build();
}

//////////////////////////////////////////////////////////////////////
//
// ControlFlowGraph
//
ControlFlowGraph::ControlFlowGraph(Function* function)
    : Graph(this), function_(function) {
}

// Graph::Provider
Value* ControlFlowGraph::entry() {
  return function_->entry_block();
}

std::vector<Value*> ControlFlowGraph::predecessors_of(Value* value) {
  std::vector<Value*> predecessors;
  for (auto predecessor : value->as<BasicBlock>()->predecessors())
    predecessors.push_back(predecessor);
  return predecessors;
}

std::vector<Value*> ControlFlowGraph::successors_of(Value* value) {
  std::vector<Value*> successors;
  for (auto successor : value->as<BasicBlock>()->successors())
    successors.push_back(successor);
  return successors;
}

bool ControlFlowGraph::HasMoreThanOnePredecessors(Value* value) {
  auto count = 0;
  for (auto predecessor : value->as<BasicBlock>()->predecessors()) {
    DCHECK(predecessor);
    ++count;
    if (count == 2)
      return true;
  }
  return false;
}

}  // namespace hir
}  // namespace elang
