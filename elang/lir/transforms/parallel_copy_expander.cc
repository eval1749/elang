// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/parallel_copy_expander.h"

#include "base/logging.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions_.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
bool IsImmediate(Value value) {
  return !value.is_physical() && !value.is_virtual() &&
         value.kind != Value::Kind::Argument &&
         value.kind != Value::Kind::Parameter && !value.is_stack_slot();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander
//
ParallelCopyExpander::ParallelCopyExpander(Factory* factory, Value type)
    : FactoryUser(factory),
      number_of_required_scratches(0),
      number_of_scratches(0),
      type_(type) {
}

ParallelCopyExpander::~ParallelCopyExpander() {
}

void ParallelCopyExpander::AddScratch(Value scratch) {
  DCHECK(scratch.is_physical());
  DCHECK(scratch.type == type.type);
  if (scratch1_.is_void()) {
    DCHECK_NE(scratch1_, scratch);
    scratch1_ = scratch;
    return;
  }
  DCHECK_NE(scratch1_, scratch);
  DCHECK(scratch2_.is_void());
  scratch2_ = scratch;
}

void ParallelCopyExpander::AddTask(Value output, Value input) {
  DCHECK_NE(output, input);
  DCHECK_EQ(output.type, type_.type);
  DCHECK_EQ(input.type, type_.type);
  DCHECK_EQ(output.size, input.size);

  if (!IsImmediate(input))
    dependency_graph_.AddEdge(input, output);
  tasks_.push_back({output, input});
}

void ParallelCopyExpander::EmitCopy(Value output, Value input) {
  if (input.is_physical()) {
    instructions_.push_back(factory()->NewCopylInstruction(output, input));
    return;
  }
  if (Target::HasCopyImmediateToMemory(type_) && IsImmediate(input)) {
    instructions_.push_back(factory()->NewLiteralInstruction(output, input));
    return;
  }
  EmiCopy(scratch1_, input);
  EmiCopy(output, scratch1_);
}

Value ParallelCopyExpander::EmitSwap(Value output, Value input) {
  dependency_graph_.RemoveEdge(output, input);
  if (!output.is_physical() || !input.is_physical()) {
    if (output.is_physical() || input.is_physical()) {
      EmitCopy(output, input);
      return input;
    }
    EmitCopy(scratch1_, input);
    EmitCopy(scratch2_, output);
    EmitCopy(input, scratch2_);
    EmitCopy(output, scratch1_);
    return scratch2_;
  }

  DCHECK(output.is_physical() && input.is_physical());
  if (Target::HasSwapInstruction(type_)) {
    instructions_.push_back(factory()->NewSwapInstruciton(output, input));
    return input;
  }
  EmitCopy(scratch1_, input);
  EmitCopy(input, output);
  EmitCopy(output, scratch1_);
  return input;
}

std::vector<Instruction*> ParallelCopyExpander::Plan() {
  if (!Prepare())
    return {};
  DCHECK(instructions_.empty());
  while (!tasks_.empty()) {
    std::vector<Task> pending_tasks;
    for (auto const& task : tasks) {
      if (dependency_graph_.HasInEdge(task.output)) {
        pending_tasks.push_back(task);
        continue;
      }
      EmitCopy(task.output, task.input);
    }
    if (pending_tasks.empty())
      break;
    DCHECK_GE(pending_tasks.size(), 2u);

    // Emit swap for one task and rewrite rest of tasks using swapped output.
    auto const swap = pending_tasks.back();
    pending_tasks.pop_back();
    auto const new_output = EmitSwap(swap.output, swap.input);
    tasks_ = std::move(pending_tasks);
    for (auto& task : tasks_) {
      if (task.input != swap.output)
        continue;
      // Rewrite task to use new input.
      dependency_graph_.RemoveEdge(task.output, task.input);
      dependency_graph_.AddEdge(task.output, new_output);
      task.input = new_output;
    }
  }
  return std::move(instructions_);
}

bool ParallelCopyExpander::Prepare() {
  if (scratch2_.is_physical())
    return true;

  auto number_of_scratches = scratch1_.is_physical() ? 1 : 0;
  auto number_of_required_scratches = 0;

  for (auto const& task : tasks_) {
    auto const output = task->output;
    if (output.is_physical()) {
      if (dependency_graph_.HasInEdge(output))
        continue;
      if (scratch1_.is_void()) {
        DCHECK_EQ(number_scratches_, 0);
        scratch1_ = output;
        number_of_scratches = 1;
        continue;
      }
      // Since, we can do all variation of copy by two scratch registers,
      // we don't need to check rest of tasks.
      DCHECK_EQ(number_scratches_, 1);
      scratch2_ = output;
      number_of_scratches = 2;
      DCHECK_NE(scratch1_, scratch2_);
      break;
    }

    if (number_of_required_scratches == 2)
      continue;

    auto const input = task->input;
    if (input.is_physical()) {
      number_of_required_scratches = 1;
      continue;
    }
    if (IsImmediate(input)) {
      if (Target::HasCopyImmediateToMemory(input))
        continue;
      number_of_required_scratches = 1;
      continue;
    }
    if (!dependency_graph_.HasInEdge(output)) {
      number_of_required_scratches = 1;
      continue;
    }
    // Memory rotation requires two scratch registers.
    number_of_required_scratches = 2;
  }

  return number_of_scratches <= number_of_required_scratches;
}

}  // namespace lir
}  // namespace elang
