// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_DATA_FLOW_SOLVER_H_
#define ELANG_BASE_ANALYSIS_DATA_FLOW_SOLVER_H_

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_editor.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/base/graphs/graph.h"
#include "elang/base/graphs/graph_sorter.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// DataFlowSolver
//
template <typename Graph, typename Variable>
class DataFlowSolver final
    : public LivenessEditor<typename Graph::Derived*, Variable> {
 public:
  typedef typename Graph::Derived GraphNode;
  typedef LivenessCollection<GraphNode*, Variable> Collection;

  DataFlowSolver(const Graph* graph, Collection* collection)
      : LivenessEditor(collection), collection_(collection), graph_(graph) {}

  ~DataFlowSolver() = default;

  void SolveBackward();

 private:
  Collection* const collection_;
  const Graph* const graph_;

  DISALLOW_COPY_AND_ASSIGN(DataFlowSolver);
};

// Solve data flow in backward to store In and Out. Out must be empty before
// solving.
template <typename Graph, typename Variable>
void DataFlowSolver<Graph, Variable>::SolveBackward() {
  DCHECK(collection_->LivenessOf(graph_->first_node()).in().IsEmpty())
      << "In(entry) should be empty.";
#ifndef NDEBUG
  for (auto node : graph_->nodes()) {
    DCHECK(collection_->LivenessOf(node).out().IsEmpty())
        << "Out(*) should be empty.";
  }
#endif
  auto const work = this->work();
  auto counter = 0;
  auto changed = true;
  while (changed) {
    ++counter;
    DCHECK_LT(counter, 10000) << "Too complex graph?";
    changed = false;
    for (auto const node : Graph::Sorter::SortByReversePreOrder(graph_)) {
      auto const liveness = Edit(node);
      for (auto const successor : node->successors())
        EditOut(liveness)->Union(LivenessOf(successor).in());
      // in = out - kill | in
      work->CopyFrom(liveness->out());
      work->Subtract(liveness->kill());
      work->Union(liveness->in());
      if (liveness->in().Equals(*work))
        continue;
      EditIn(liveness)->CopyFrom(*work);
      changed = true;
    }
  }
  DCHECK(collection_->LivenessOf(graph_->first_node()).in().IsEmpty())
      << "In(entry) should be empty.";
  DCHECK(collection_->LivenessOf(graph_->last_node()).out().IsEmpty())
      << "Out(exit) should be empty.";
}

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_DATA_FLOW_SOLVER_H_
