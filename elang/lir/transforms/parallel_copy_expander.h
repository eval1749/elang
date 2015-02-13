// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_
#define ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_

#include <vector>

#include "elang/base/simple_directed_graph.h"
#include "elang/lir/factory_user.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander
//
class ELANG_LIR_EXPORT ParallelCopyExpander final : public FactoryUser {
 public:
  explicit ParallelCopyExpander(Factory* factory, Value type);
  ~ParallelCopyExpander();

  // Add scratch register |scratch| for used for generating code.
  void AddScratch(Value scratch);
  void AddTask(Value output, Value input);

  std::vector<Instruction*> Plan() const;

 private:
  struct Task;

  void EmitCopy(Value output, Value input);

  // Emit swap sequence and returns a physical register which contains "old"
  // |output| value.
  Value EmitSwap(Value output, Value input);

  // Returns true if we can generate instructions for parallel copy.
  bool Prepare();

  SimpleDirectedGraph<Value> dependency_graph_;
  std::vector<Instruction*> instructions_;
  Value scratch1_;
  Value scratch2_;
  const Value type_;

  DISALLOW_COPY_AND_ASSIGN(ParallelCopyExpander);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_
