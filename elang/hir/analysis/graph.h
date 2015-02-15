// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ANALYSIS_GRAPH_H_
#define ELANG_HIR_ANALYSIS_GRAPH_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/ordered_list.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

class Function;
class Value;

//////////////////////////////////////////////////////////////////////
//
// Graph
//
class ELANG_HIR_EXPORT Graph {
 public:
  class ELANG_HIR_EXPORT Provider {
   public:
    virtual ~Provider();

    virtual Value* entry() const = 0;
    virtual bool HasMoreThanOnePredecessor(Value* value) const = 0;
    virtual std::vector<Value*> PredecessorsOf(Value* value) const = 0;
    virtual std::vector<Value*> SuccessorsOf(Value* value) const = 0;

   protected:
    Provider();

   private:
    DISALLOW_COPY_AND_ASSIGN(Provider);
  };

  virtual ~Graph();

  Value* entry() const;

  bool HasMoreThanOnePredecessor(Value* value) const;
  std::vector<Value*> PredecessorsOf(Value* value) const;
  std::vector<Value*> SuccessorsOf(Value* value) const;

  // For clean control flow graph
  OrderedList<Value*> PostOrderList() const;
  OrderedList<Value*> PreOrderList() const;
  // For dominator tree
  OrderedList<Value*> ReversePostOrderList() const;
  OrderedList<Value*> ReversePreOrderList() const;

 protected:
  explicit Graph(Provider* provider);

 private:
  Provider* const provider_;

  DISALLOW_COPY_AND_ASSIGN(Graph);
};

//////////////////////////////////////////////////////////////////////
//
// ControlFlowGraph
//
class ELANG_HIR_EXPORT ControlFlowGraph : public Graph, public Graph::Provider {
 public:
  explicit ControlFlowGraph(Function* function);

  // Graph::Provider
  Value* entry() const final;
  std::vector<Value*> PredecessorsOf(Value* value) const final;
  std::vector<Value*> SuccessorsOf(Value* value) const final;
  bool HasMoreThanOnePredecessor(Value* value) const final;

 private:
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(ControlFlowGraph);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ANALYSIS_GRAPH_H_
