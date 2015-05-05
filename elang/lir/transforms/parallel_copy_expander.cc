// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <list>
#include <memory>
#include <ostream>
#include <unordered_set>
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

bool IsMemory(Value value) {
  return value.is_argument() || value.is_parameter() || value.is_stack_slot();
}

bool IsRegister(Value value) {
  return value.is_physical();
}

bool IsImmediate(Value value) {
  return !IsRegister(value) && !IsMemory(value);
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

  static bool Less(const Task& task1, const Task& task2);
  static int OrderOf(const Task task);

  bool operator==(const Task& other) const {
    return output == other.output && input == other.input;
  }

  bool operator!=(const Task& other) const { return !operator==(other); }
};

std::ostream& operator<<(std::ostream& ostream,
                         const ParallelCopyExpander::Task& task) {
  return ostream << "Task(" << task.output << " <- " << task.input << ")";
}

bool ParallelCopyExpander::Task::Less(const Task& task1, const Task& task2) {
  if (auto const diff = Task::OrderOf(task1) - Task::OrderOf(task2))
    return diff < 0;
  return task1.output.data < task2.output.data;
}

// The order of task:
//   1. break register to memory by using scratch
//   2. break register to register by using swap
//   3. break memory to memory by using scratch
//   4. break memory to register by using scratch
//   5. Immediate to register/memory
int ParallelCopyExpander::Task::OrderOf(Task task) {
  if (IsRegister(task.input))
    return IsMemory(task.output) ? 1 : 2;
  if (IsMemory(task.input))
    return IsMemory(task.output) ? 3 : 4;
  return 5;
}

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
  DCHECK(!output.is_virtual());
  DCHECK(!input.is_virtual());
  if (output == input)
    return;
  DCHECK_NE(output, input);
  DCHECK(!input.is_void());
  DCHECK_EQ(output.type, type_.type);
  DCHECK_EQ(output.size, input.size);
  DCHECK_EQ(input.type, type_.type);
  tasks_.push_back({output, input});
}

void ParallelCopyExpander::DidCopy(Value output, Value input) {
  dependency_graph_->RemoveEdge(output, input);
  if (!IsRegister(input) || IsSourceOfTask(input))
    return;
  scratches_.push_back(input);
}

bool ParallelCopyExpander::EmitCopy(Value output, Value input) {
  DCHECK(!input.is_void());
  DCHECK(!output.is_void());
  DCHECK_NE(output, input);
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

bool ParallelCopyExpander::EmitSwap(Task task) {
  auto const output = task.output;
  auto const input = MapInput(task.input);
  DCHECK(!IsImmediate(output));
  DCHECK(!IsImmediate(input));
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
    return true;
  }

  if (input.is_physical()) {
    // Swap physical register and memory.
    auto const scratch2 = TakeScratch(output);
    if (scratch2.is_void())
      return false;
    MustEmitCopy(output, input);
    MustEmitCopy(input, scratch2);
    // Release scratch register for |output| since caller will replace all
    // reference of |output| in tasks to |input|.
    if (!IsSourceOfTask(output)) {
      GiveScratchFor(output);
      return true;
    }
    scratch_map_.erase(scratch_map_.find(output));
    scratch_map_[input] = scratch2;
    return true;
  }

  // Swap memory operands.
  auto const scratch1 = TakeScratch(input);
  if (scratch1.is_void())
    return false;
  auto const scratch2 = TakeScratch(output);
  if (scratch2.is_void())
    return false;
  MustEmitCopy(output, scratch1);
  MustEmitCopy(input, scratch2);
  // Release scratch register for |output| since caller will replace all
  // reference of |output| in tasks to |input|.
  if (IsSourceOfTask(output)) {
    scratch_map_.erase(scratch_map_.find(output));
    scratch_map_[input] = scratch2;
    scratches_.push_back(scratch1);
    return true;
  }
  GiveScratchFor(output);
  return true;
}

// Process tasks by following steps:
//  1. Sort tasks by |Task::OrderOf(Task)|.
//  2. Build dependency graph to identify output/input dependency
//  3. Emit instructions for broken cycle task
//  4. Emit swap to break cycle
//  5 Rewrite rest of tasks using swapped output.
//  6. Emit instructions for free tasks
std::vector<Instruction*> ParallelCopyExpander::Expand() {
  DCHECK(HasTasks()) << "Please don't call |Expand()| without tasks.";
  ScopedExpand expand_scope(this);
  std::vector<Task> copy_tasks;
  std::vector<Task> free_tasks;
  std::unordered_set<Value> outputs;
  std::vector<Task> scratch_candidates;

  // Sort tasks to process memory operand first.
  std::sort(tasks_.begin(), tasks_.end(), Task::Less);

  // Step 1: Build dependency graph for tracking usage of output.
  for (auto const task : tasks_) {
    auto const output = task.output;
    DCHECK(!outputs.count(output)) << output << " is used more than once.";
    dependency_graph_->AddEdge(output, task.input);
    outputs.insert(output);
  }

  // Step 2: Collect scratch registers. We can use a register as scratch if it
  // is not input of another task, input of task is immediate or memory.
  for (auto const task : tasks_) {
    if (outputs.count(task.input) || NeedRegister(task)) {
      copy_tasks.push_back(task);
      continue;
    }
    free_tasks.push_back(task);
    if (task.output.is_physical() && !IsSourceOfTask(task.output))
      scratches_.push_back(task.output);
    if (!task.input.is_physical())
      continue;
    if (dependency_graph_->GetInEdges(task.input).size() >= 2u)
      continue;
    free_tasks.pop_back();
    scratch_candidates.push_back(task);
  }

  while (!copy_tasks.empty()) {
    std::vector<Task> pending_tasks;
    // Step 3: Expand cycle resolved tasks
    for (auto const& task : copy_tasks) {
      if (IsSourceOfTask(task.output)) {
        pending_tasks.push_back(task);
        continue;
      }
      if (EmitCopy(task.output, task.input)) {
        DidCopy(task.output, task.input);
        continue;
      }
      if (pending_tasks.empty())
        return {};
      pending_tasks.push_back(task);
    }
    if (pending_tasks.empty())
      break;
    if (pending_tasks.size() == 1u) {
      copy_tasks = pending_tasks;
      continue;
    }

    // Step 4: Emit swap for one task to break cycle.
    Task swapped = TrySwap(pending_tasks);
    if (swapped.output.is_void()) {
      while (!scratch_candidates.empty()) {
        auto const scratch = scratch_candidates.back();
        scratch_candidates.pop_back();
        MustEmitCopy(scratch.output, scratch.input);
        DidCopy(scratch.output, scratch.input);
        free_tasks.push_back({scratch.input, scratch.output});
        swapped = TrySwap(pending_tasks);
        if (!swapped.output.is_void())
          break;
      }
      if (swapped.output.is_void())
        return {};
    }

    // Step 5 Rewrite rest of tasks using swapped output.
    copy_tasks.clear();
    auto const input = MapInput(swapped.input);
    for (auto& task : pending_tasks) {
      if (task == swapped)
        continue;
      auto const last = instructions_.back();
      if (task.input != swapped.output) {
        if (last->is<CopyInstruction>() && task.output == last->output(0))
          instructions_.pop_back();
        copy_tasks.push_back(task);
        continue;
      }
      // Rewrite task to use new input.
      dependency_graph_->RemoveEdge(task.output, task.input);
      if (task.output == swapped.input)
        continue;
      if (last->is<CopyInstruction>() && task.output == last->output(0))
        instructions_.pop_back();
      copy_tasks.push_back({task.output, input});
      dependency_graph_->AddEdge(task.output, input);
    }

    if (input != swapped.input && !IsSourceOfTask(input)) {
      DCHECK(input.is_physical());
      GiveScratchFor(swapped.input);
    }
  }

  // Step 6: Expand free tasks, e.g. load immediate to physical register,
  // load memory content to physical register.
  free_tasks.insert(free_tasks.end(), scratch_candidates.begin(),
                    scratch_candidates.end());
  std::sort(free_tasks.begin(), free_tasks.end(), Task::Less);
  for (auto const task : free_tasks) {
    if (task.input.is_void())
      continue;
    auto const input = MapInput(task.input);
    if (task.output == input)
      continue;
    MustEmitCopy(task.output, input);
  }
  return std::move(instructions_);
}

void ParallelCopyExpander::GiveScratchFor(Value source) {
  auto const it = scratch_map_.find(source);
  DCHECK(it != scratch_map_.end());
  scratches_.push_back(it->second);
  scratch_map_.erase(it);
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

bool ParallelCopyExpander::NeedRegister(Task task) const {
  if (task.output.is_physical())
    return false;
  DCHECK(!task.input.is_void());
  if (task.input.is_physical()) {
    DCHECK(dependency_graph_->HasInEdge(task.input))
        << "We must have edge output to input.";
    return dependency_graph_->GetInEdges(task.input).size() >= 2u;
  }
  return !IsImmediate(task.input) ||
         !Target::HasCopyImmediateToMemory(task.input);
}

void ParallelCopyExpander::PrintTo(std::ostream* ostream) const {
  *ostream << std::endl
           << "ParallelCopyExpander:" << std::endl;
  {
    *ostream << "Scratch: {";
    auto separator = "";
    for (auto const value : scratches_) {
      *ostream << separator << value;
      separator = ", ";
    }
    *ostream << "}" << std::endl;
  }
  {
    *ostream << "Tasks: {" << std::endl;
    auto separator = "";
    for (auto const task : tasks_) {
      *ostream << "  " << task << std::endl;
      separator = ", ";
    }
    *ostream << "}" << std::endl;
  }
  {
    *ostream << "Instructions: {" << std::endl;
    auto separator = "";
    for (auto const instruction : instructions_) {
      *ostream << "  " << *instruction << std::endl;
      separator = ", ";
    }
    *ostream << "}" << std::endl;
  }
  *ostream << "Dependency:" << std::endl;
  for (auto const node : dependency_graph_->GetAllVertices()) {
    *ostream << "  " << node << " {";
    auto separator = "";
    for (auto const user : dependency_graph_->GetInEdges(node)) {
      *ostream << separator << user;
      separator = ", ";
    }
    *ostream << "}" << std::endl;
  }
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

ParallelCopyExpander::Task ParallelCopyExpander::TrySwap(
    const std::vector<Task>& tasks) {
  for (auto const task : tasks) {
    if (EmitSwap(task)) {
      dependency_graph_->RemoveEdge(task.output, task.input);
      return task;
    }
  }
  return Task();
}

std::ostream& operator<<(std::ostream& ostream,
                         const ParallelCopyExpander& expander) {
  expander.PrintTo(&ostream);
  return ostream;
}

}  // namespace lir
}  // namespace elang
