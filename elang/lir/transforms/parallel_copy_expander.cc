// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/parallel_copy_expander.h"

#include "base/logging.h"
#include "elang/base/zone_allocated.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
bool IsImmediate(Value value) {
  return !value.is_physical() && !value.is_virtual() &&
         value.kind != Value::Kind::Argument &&
         value.kind != Value::Kind::Parameter &&
         !value.is_stack_slot();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander::Task
//
struct ParallelCopyExpander::Task : ZoneAllocated {
  Value output;
  Value input;

  Task(Value output, Value input) : output(output), input(input) {}
};

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander
//
ParallelCopyExpander::ParallelCopyExpander(Editor* editor,
                                           Value type,
                                           Instruction* ref_instr)
    : EditorUser(editor), ref_instr_(ref_instr), type_(type) {
}

ParallelCopyExpander::~ParallelCopyExpander() {
}

void ParallelCopyExpander::AddTask(Value output, Value input) {
  DCHECK_NE(output, input);
  DCHECK_EQ(output.type, type_.type);
  DCHECK_EQ(input.type, type_.type);
  DCHECK_EQ(output.size, input.size);

  if (!IsImmediate(input))
    dependency_graph_.AddEdge(input, output);

#define ENCODE_KINDS(output, input) \
  ((static_cast<int>(output) << 8) | static_cast<int>(input))

#define ENCODED_CASE(output, input) \
  ENCODE_KINDS(Value::Kind::output, Value::Kind::input)

  auto const task = new(zone()) Task(output, input);

  switch (ENCODE_KINDS(output.kind, input.kind)) {
    case ENCODED_CASE(PhysicalRegister, PhysicalRegister):
      register_to_register_tasks_.push_back(task);
      return;
    case ENCODED_CASE(PhysicalRegister, Argument):
    case ENCODED_CASE(PhysicalRegister, Parameter):
    case ENCODED_CASE(PhysicalRegister, StackSlot):
      memory_to_register_tasks_.push_back(task);
      return;
    case ENCODED_CASE(PhysicalRegister, Immediate):
    case ENCODED_CASE(PhysicalRegister, Literal):
      immediate_to_register_tasks_.push_back(task);
      return;
    case ENCODED_CASE(Argument, PhysicalRegister):
    case ENCODED_CASE(Parameter, PhysicalRegister):
    case ENCODED_CASE(StackSlot, PhysicalRegister):
      register_to_memory_tasks_.push_back(task);
      return;
    case ENCODED_CASE(Argument, Immediate):
    case ENCODED_CASE(Argument, Literal):
    case ENCODED_CASE(Parameter, Immediate):
    case ENCODED_CASE(Parameter, Literal):
    case ENCODED_CASE(StackSlot, Immediate):
    case ENCODED_CASE(StackSlot, Literal):
      immediate_to_memory_tasks_.push_back(task);
      return;
    case ENCODED_CASE(Argument, Argument):
    case ENCODED_CASE(Argument, Parameter):
    case ENCODED_CASE(Argument, StackSlot):
    case ENCODED_CASE(Parameter, Argument):
    case ENCODED_CASE(Parameter, Parameter):
    case ENCODED_CASE(Parameter, StackSlot):
    case ENCODED_CASE(StackSlot, Argument):
    case ENCODED_CASE(StackSlot, Parameter):
    case ENCODED_CASE(StackSlot, StackSlot):
      memory_to_memory_tasks_.push_back(task);
      return;
  }
  NOTREACHED() << "unsupported copy " << output << " " << input;
}

void ParallelCopyExpander::EmitCopy(Value output, Value input) {
  editor()->InsertCopyBefore(output, input, ref_instr_);
}

void ParallelCopyExpander::Expand() {
  editor()->Edit(ref_instr_->basic_block());
  ExpandMemoryToMemoryCopy();
  editor()->Commit();
}

void ParallelCopyExpander::ExpandMemoryToMemoryCopy() {
  for (;;) {
    std::vector<Task*> pending_tasks;
    for (auto task : memory_to_memory_tasks_) {
      auto const output = task->output;
      if (dependency_graph_.HasInEdge(output)) {
        pending_tasks.push_back(task);
        continue;
      }
      EmitCopy(scratch1_, task->input);
      EmitCopy(task->output, scratch1_);
      for (auto user : dependency_graph_.GetOutEdges(output))
        dependency_graph_.RemoveEdge(output, user);
    }

    if (pending_tasks.empty())
      break;

    DCHECK_GE(pending_tasks.size(), 2u);

    auto const swap = pending_tasks.back();
    pending_tasks.pop_back();

    // To expand |pcopy A, B = B, A|, swap, we use two scratch registers.
    EmitCopy(scratch1_, swap->input);
    EmitCopy(scratch2_, swap->output);
    EmitCopy(swap->input, scratch2_);
    EmitCopy(swap->output, scratch1_);
    dependency_graph_.RemoveEdge(swap->output, swap->input);
    dependency_graph_.RemoveEdge(swap->input, swap->output);

    for (auto const pending_task : pending_tasks) {
      if (pending_task->input != swap->output)
        continue;
      dependency_graph_.RemoveEdge(pending_task->output, pending_task->input);
      pending_task->input = swap->input;
      dependency_graph_.AddEdge(pending_task->output, pending_task->input);
    }
  }
}

bool ParallelCopyExpander::NeedPhysicalRegister() const {
  if (memory_to_memory_tasks_.empty()) {
    if (Target::HasCopyImmediateToMemory())
      return false;
    if (immediate_to_memory_tasks_.empty())
      return false;
  }

  if (immediate_to_memory_tasks_.empty() || Target::HasCopyImmediateToMemory())
    return false;

  if (!memory_to_register_tasks_.empty() ||
      !immediate_to_register_tasks_.empty()) {
    return false;
  }

  // Find a register to use.
  std::unordered_set<Value> candidates;
  for (auto const task : immediate_to_register_tasks_)
    candidates.insert(task->output);
  for (auto const task : register_to_register_tasks_)
    candidates.erase(task->input);
  return candidates.empty();
}

}  // namespace lir
}  // namespace elang
