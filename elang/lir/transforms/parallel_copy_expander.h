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
// ParallelCopyExpander expands parallel copy to sequence of copy, literal,
// swap, and xor instructions. ParallelCopyExpander uses give scratch register
// if needed or fail. ParallelCopyExpander requires at most two scratch
// registers if copy tasks containing memory rotation, e.g. A <- B <- C <- A.
//
// Memory Operands Examples:
//  # Swap
//  pcopy A, B = B, A
//  =>
//  load R1 = A
//  load R2 = B
//  store A = R1
//  store B = R2
//
//  # Rotate
//  pcopy A, B, C = B, C, A
//  =>
//  load R1 = A
//  load R2 = C
//  store C = R1  ; C = A
//  load R1 = B
//  store A = R1  ; A = B
//  store B = R2  ; B = A
//
//
class ELANG_LIR_EXPORT ParallelCopyExpander final : public FactoryUser {
 public:
  explicit ParallelCopyExpander(Factory* factory, Value type);
  ~ParallelCopyExpander();

  // Add scratch register to use. This function must be called |HasTask()|
  // returns true.
  void AddScratch(Value physical);

  // Add a task copying |input| to |output|. |output| should be either physical
  // register or memory operand. |input| can be one of physical register,
  // immediate, or memory operand.
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

  // Release scratch register containing value of |input|.
  void GiveScratch(Value input);

  // Release scratch register containing value of |input| if |input| isn't
  // source of pending task.
  void GiveScratchIfNotUsed(Value input);

  // Returns true if |task| doesn't need to use other registers to complete.
  bool IsFreeTask(Task task) const;

  // Returns true if |value| is a source of pending task.
  bool IsSourceOfTask(Value value) const;

  // Returns physical register containing value of |input| or |input| itself.
  Value MapInput(Value input) const;

  // Emit instructions for copying |input| to |output|.
  void MustEmitCopy(Value output, Value input);

  // Returns scratch register holding |input| or void if scratch registers are
  // unavailable.
  Value TakeScratch(Value input);

  // Tracking task source by edge from using value to used value, e.g.
  // output to input edge.
  SimpleDirectedGraph<Value>* dependency_graph_;

  // Holds result of expansion.
  std::vector<Instruction*> instructions_;

  // List of available scratch registers.
  std::vector<Value> scratches_;

  // Map to scratch register containing value of key.
  std::unordered_map<Value, Value> scratch_map_;

  // List of copy tasks.
  std::vector<Task> tasks_;

  // A type of copy task operands.
  const Value type_;

  DISALLOW_COPY_AND_ASSIGN(ParallelCopyExpander);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_
