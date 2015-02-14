// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "elang/lir/transforms/parallel_copy_expander.h"

#include "base/logging.h"
#include "elang/base/simple_directed_graph.h"
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
         value.kind != Value::Kind::Parameter && !value.is_stack_slot();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander::ScopedExpand
//
class ParallelCopyExpander::ScopedExpand {
 public:
  const std::unique_ptr<SimpleDirectedGraph<Value>> dependency_graph_;
  ParallelCopyExpander* expander_;

  explicit ScopedExpand(ParallelCopyExpander* expander)
      : dependency_graph_(new SimpleDirectedGraph<Value>()),
        expander_(expander) {
    DCHECK(expander_->instructions_.empty());
    DCHECK(expander_->scratches_.empty());
    DCHECK(expander_->scratch_map_.empty());
    DCHECK(!expander_->dependency_graph_);
    expander_->dependency_graph_ = dependency_graph_.get();
  }

  ~ScopedExpand() {
    expander_->dependency_graph_ = nullptr;
    expander_->instructions_.clear();
    expander_->scratches_.clear();
    expander_->scratch_map_.clear();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ScopedExpand);
};

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander::Task
//
struct ParallelCopyExpander::Task {
  Value output;
  Value input;
};

//////////////////////////////////////////////////////////////////////
//
// ParallelCopyExpander
//
ParallelCopyExpander::ParallelCopyExpander(Factory* factory, Value type)
    : FactoryUser(factory), dependency_graph_(nullptr), type_(type) {
}

ParallelCopyExpander::~ParallelCopyExpander() {
}

void ParallelCopyExpander::AddScratch(Value scratch) {
  DCHECK(HasTasks()) << "Please add a task before adding scratch register.";
  DCHECK(scratch.is_physical());
  DCHECK_EQ(scratch.type, type_.type);
  tasks_.push_back({scratch, Value()});
}

void ParallelCopyExpander::AddTask(Value output, Value input) {
  if (output == input)
    return;
  DCHECK_NE(output, input);
  DCHECK(!input.is_void());
  DCHECK_EQ(output.type, type_.type);
  DCHECK_EQ(output.size, input.size);
  DCHECK_EQ(input.type, type_.type);
  tasks_.push_back({output, input});
}

bool ParallelCopyExpander::EmitCopy(Value output, Value input) {
  DCHECK(!input.is_void());
  DCHECK(!output.is_void());
  if (input.is_physical()) {
    instructions_.push_back(factory()->NewCopyInstruction(output, input));
    return true;
  }
  if (IsImmediate(input)) {
    if (output.is_physical() || Target::HasCopyImmediateToMemory(type_)) {
      instructions_.push_back(factory()->NewLiteralInstruction(output, input));
      return true;
    }
    auto const scratch = TakeScratch(input);
    if (!scratch.is_physical())
      return false;
    MustEmitCopy(output, scratch);
    return true;
  }
  if (output.is_physical()) {
    instructions_.push_back(factory()->NewCopyInstruction(output, input));
    return true;
  }
  auto const scratch = TakeScratch(input);
  if (!scratch.is_physical())
    return false;
  MustEmitCopy(output, scratch);
  return true;
}

bool ParallelCopyExpander::EmitSwap(Value output, Value source) {
  DCHECK(!IsImmediate(output));
  DCHECK(!IsImmediate(source));
  dependency_graph_->RemoveEdge(output, source);
  auto const input = MapInput(source);
  if (output.is_physical() && input.is_physical()) {
    // Swap two physical registers
    if (Target::HasSwapInstruction(type_)) {
      instructions_.push_back(
          factory()->NewPCopyInstruction({output, input}, {input, output}));
      return true;
    }
    auto const scratch = TakeScratch(input);
    if (scratch.is_void()) {
      if (!Target::HasXorInstruction(input))
        return false;
      // Scratch register free, two operands arithmetic compatible register
      // swap:
      //    xor a = b  ; a := a ^ b
      //    xor b = a  ; b := b ^ a = (a ^ b ^ b) = a
      //    xor a = b  ; a := a ^ b = (a ^ b ^ a) = b
      instructions_.push_back(
          factory()->NewBitXorInstruction(output, output, input));
      instructions_.push_back(
          factory()->NewBitXorInstruction(input, input, output));
      instructions_.push_back(
          factory()->NewBitXorInstruction(output, output, input));
      return true;
    }
    MustEmitCopy(input, output);
    MustEmitCopy(output, scratch);
    return true;
  }

  if (output.is_physical()) {
    // Swap physical register and memory.
    auto const scratch = TakeScratch(input);
    if (scratch.is_void())
      return false;
    MustEmitCopy(input, output);
    MustEmitCopy(output, scratch);
    GiveScratchIfNotUsed(input);
    return true;
  }

  if (input.is_physical()) {
    // Swap physical register and memory.
    auto const scratch = TakeScratch(output);
    if (scratch.is_void())
      return false;
    MustEmitCopy(output, input);
    MustEmitCopy(input, scratch);
    // Release scratch register for |output| since caller will replace all
    // reference of |output| in tasks to |input|.
    GiveScratch(output);
    return true;
  }

  // Swap memory operands.
  auto const scratch1 = TakeScratch(input);
  if (scratch1.is_void())
    return false;
  auto const scratch2 = TakeScratch(output);
  if (scratch2.is_void())
    return false;
  MustEmitCopy(input, scratch2);
  MustEmitCopy(output, scratch1);
  // Release scratch register for |output| since caller will replace all
  // reference of |output| in tasks to |input|.
  GiveScratch(output);
  if (IsSourceOfTask(output)) {
    scratch_map_[input] = scratch1;
    return true;
  }
  GiveScratchIfNotUsed(input);
  return true;
}

std::vector<Instruction*> ParallelCopyExpander::Expand() {
  DCHECK(HasTasks()) << "Please don't call |Expand()| without tasks.";
  ScopedExpand expand_scope(this);
  std::vector<Task> copy_tasks;
  std::vector<Task> free_tasks;

  // Step 1: Build dependency graph for tracking usage of output.
  for (auto const task : tasks_)
    dependency_graph_->AddEdge(task.output, task.input);

  // Step 2: Collect scratch registers. We can use a register as scratch if it
  // is not input of another task, input of task is immediate or memory.
  for (auto const task : tasks_) {
    if (!IsFreeTask(task)) {
      copy_tasks.push_back(task);
      continue;
    }
    free_tasks.push_back(task);
    if (!task.output.is_physical() || IsSourceOfTask(task.output))
      continue;
    scratches_.push_back(task.output);
  }

  // Step 3: Expand copy tasks
  while (!copy_tasks.empty()) {
    std::vector<Task> pending_tasks;
    for (auto const& task : copy_tasks) {
      if (IsSourceOfTask(task.output)) {
        pending_tasks.push_back(task);
        continue;
      }
      if (!IsImmediate(task.input))
        dependency_graph_->RemoveEdge(task.output, task.input);
      if (!EmitCopy(task.output, task.input))
        return {};
    }
    if (pending_tasks.empty())
      break;
    DCHECK_GE(pending_tasks.size(), 2u);

    // Emit swap for one task and rewrite rest of tasks using swapped output.
    auto const swap = pending_tasks.back();
    pending_tasks.pop_back();
    if (!EmitSwap(swap.output, swap.input))
      return {};
    copy_tasks.clear();
    for (auto& task : pending_tasks) {
      if (task.input != swap.output) {
        copy_tasks.push_back(task);
        continue;
      }
      // Rewrite task to use new input.
      dependency_graph_->RemoveEdge(task.output, task.input);
      if (task.output == swap.input)
        continue;
      auto const input = MapInput(swap.input);
      copy_tasks.push_back({task.output, input});
      dependency_graph_->AddEdge(task.output, input);
    }
  }

  // Step 4: Expand free tasks, e.g. load immediate to physical register,
  // load memory content to physical register.
  for (auto const task : free_tasks) {
    if (task.input.is_void())
      continue;
    EmitCopy(task.output, task.input);
  }
  return std::move(instructions_);
}

void ParallelCopyExpander::GiveScratch(Value source) {
  auto const it = scratch_map_.find(source);
  DCHECK(it != scratch_map_.end());
  scratches_.push_back(it->second);
  scratch_map_.erase(it);
}

void ParallelCopyExpander::GiveScratchIfNotUsed(Value source) {
  if (IsSourceOfTask(source))
    return;
  GiveScratch(source);
}

bool ParallelCopyExpander::IsFreeTask(Task task) const {
  if (task.input.is_void()) {
    DCHECK(task.output.is_physical()) << "Bad scratch register";
    return true;
  }
  if (IsSourceOfTask(task.input))
    return false;
  if (task.input.is_physical())
    return true;
  if (IsImmediate(task.input))
    return Target::HasCopyImmediateToMemory(task.input);
  return false;
}

bool ParallelCopyExpander::IsSourceOfTask(Value value) const {
  return dependency_graph_->HasInEdge(value);
}

Value ParallelCopyExpander::MapInput(Value source) const {
  auto const it = scratch_map_.find(source);
  if (it != scratch_map_.end())
    return it->second;
  return source;
}

void ParallelCopyExpander::MustEmitCopy(Value output, Value input) {
  auto const succeeded = EmitCopy(output, input);
  DCHECK(succeeded) << "EmitCopy(" << output << ", " << input << ") failed";
}

Value ParallelCopyExpander::TakeScratch(Value source) {
  if (scratches_.empty())
    return Value();
  auto const scratch = scratches_.back();
  DCHECK(scratch.is_physical());
  MustEmitCopy(scratch, source);
  scratches_.pop_back();
  DCHECK(!scratch_map_.count(source));
  scratch_map_[source] = scratch;
  return scratch;
}

}  // namespace lir
}  // namespace elang
