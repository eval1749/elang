// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_
#define ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_

#include <vector>

#include "elang/lir/factory_user.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {

template <typename Node>
class SimpleDirectedGraph;

namespace lir {

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander
//
// Copying memory-to-memory requires one scratch registers.
// Swapping memory and memory requires two scratch registers.
//
class ELANG_LIR_EXPORT ParallelCopyExpander final : public FactoryUser {
 public:
  explicit ParallelCopyExpander(Factory* factory, Value type);
  ~ParallelCopyExpander();

  void AddScratch(Value physical);
  void AddTask(Value output, Value input);

  // Returns instructions for executing parallel if no additional registers are
  // required, otherwise returns empty vector. You should add scratch register
  // if this function returns empty vector.
  std::vector<Instruction*> Expand();

  // Returns true if this expander has at least one task.
  bool HasTasks() const { return !tasks_.empty(); }

 private:
  class ScopedExpand;
  struct Task;

  // Emit instructions for copying |input| to |output| and returns true if
  // succeeded, otherwise false. This function fails if |output| isn't a
  // physical and |input| is memory or immediate and there are no scratch
  // registers.
  bool EmitCopy(Value output, Value input);

  // Emit instructions for swapping |output| and |input| and returns true if
  // succeeded, otherwise returns false.
  bool EmitSwap(Value output, Value input);

  void GiveScratch(Value input);
  void GiveScratchIfNotUsed(Value input);
  bool IsFreeTask(Task task) const;
  bool IsSourceOfTask(Value value) const;
  Value MapInput(Value input) const;
  void MustEmitCopy(Value output, Value input);
  Value TakeScratch(Value input);

  SimpleDirectedGraph<Value>* dependency_graph_;
  std::vector<Instruction*> instructions_;

  std::vector<Value> scratches_;
  std::unordered_map<Value, Value> scratch_map_;
  std::vector<Task> tasks_;
  const Value type_;

  DISALLOW_COPY_AND_ASSIGN(ParallelCopyExpander);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_
