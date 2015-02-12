// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_
#define ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_

#include <vector>

#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/lir/editor_user.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class Editor;
class Instruction;

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander
//
class ELANG_LIR_EXPORT ParallelCopyExpander final
    : public EditorUser, public ZoneOwner {
 public:
  explicit ParallelCopyExpander(Editor* editor,
                                Value type,
                                Instruction* ref_instr);
  ~ParallelCopyExpander();

  void AddTask(Value output, Value input);

  // Expand parallel copy before |ref_instr_| with destroying contents
  // of scratch registers if needed.
  void Expand();

  // Returns true expanding parallel copy requires physical register.
  bool NeedPhysicalRegister() const;

 private:
  struct Task;

  void EmitCopy(Value output, Value input);
  void ExpandMemoryToMemoryCopy();

  SimpleDirectedGraph<Value> dependency_graph_;

  std::vector<Task*> immediate_to_memory_tasks_;
  std::vector<Task*> immediate_to_register_tasks_;
  std::vector<Task*> memory_to_memory_tasks_;
  std::vector<Task*> memory_to_register_tasks_;
  std::vector<Task*> register_to_memory_tasks_;
  std::vector<Task*> register_to_register_tasks_;

  Instruction* const ref_instr_;

  // Scratch registers
  Value scratch1_;
  Value scratch2_;

  // type
  const Value type_;

  DISALLOW_COPY_AND_ASSIGN(ParallelCopyExpander);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_PARALLEL_COPY_EXPANDER_H_
