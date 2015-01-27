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

    virtual Value* entry() = 0;
    virtual std::vector<Value*> predecessors_of(Value* value) = 0;
    virtual std::vector<Value*> successors_of(Value* value) = 0;

   protected:
    Provider();

   private:
    DISALLOW_COPY_AND_ASSIGN(Provider);
  };

  virtual ~Graph();

  Value* entry() const;
  std::vector<Value*> predecessors_of(Value* value);
  std::vector<Value*> successors_of(Value* value);

  OrderedList<Value*> ReversePostOrderList();

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
  Value* entry() final;
  std::vector<Value*> predecessors_of(Value* value) final;
  std::vector<Value*> successors_of(Value* value) final;

 private:
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(ControlFlowGraph);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ANALYSIS_GRAPH_H_
