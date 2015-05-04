// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "elang/lir/editor.h"

#include "base/logging.h"
#include "elang/base/analysis/dominator_tree_builder.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/base/graphs/graph_sorter.h"
#include "elang/lir/analysis/liveness_analyzer.h"
#include "elang/lir/analysis/conflict_map.h"
#include "elang/lir/analysis/conflict_map_builder.h"
#include "elang/lir/error_data.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/validator.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : ErrorReporter(factory),
      basic_block_(nullptr),
      factory_(factory),
      function_(function),
      graph_editor_(function),
      is_index_valid_(false) {
  counters_.block_counter = 0;
  counters_.instruction_counter = 0;
  counters_.output_counter = 0;
}

Editor::~Editor() {
  DCHECK(!basic_block_);
}

BasicBlock* Editor::entry_block() const {
  return function()->entry_block();
}

BasicBlock* Editor::exit_block() const {
  return function()->exit_block();
}

// Add edges between |instruction|'s block and new successors.
// Note: This function doesn't call |DidChangeControlFlow()| because this
// function is used for multiple edge mutation, e.g. |DidChangeControlFlow()|.
// Callers must call |DidChangeControlFlow()| to notify control flow change.
void Editor::AddEdgesFrom(Instruction* instruction) {
  if (!instruction->IsTerminator())
    return;
  auto const block = instruction->basic_block();
  for (auto successor : instruction->block_operands())
    graph_editor_.AddEdge(block, successor);
}

const Editor::LivenessData& Editor::AnalyzeLiveness() const {
  if (liveness_data_)
    return *liveness_data_;
  liveness_data_ = std::move(::elang::lir::AnalyzeLiveness(function()));
  return *liveness_data_;
}

const ConflictMap& Editor::AnalyzeConflicts() const {
  if (conflict_map_)
    return *conflict_map_;
  conflict_map_.reset(new ConflictMap(ConflictMapBuilder(this).Build()));
  return *conflict_map_;
}

Editor::Counters Editor::AssignIndex() {
  DCHECK(!basic_block_);
  if (is_index_valid_)
    return counters_;
  counters_.block_counter = 0;
  counters_.instruction_counter = 0;
  counters_.output_counter = 0;
  for (auto const block : function()->basic_blocks()) {
    block->index_ = counters_.block_counter;
    ++counters_.block_counter;
    for (auto const phi_instr : block->phi_instructions()) {
      DCHECK(phi_instr->output(0).is_virtual());
      ++counters_.output_counter;
      phi_instr->index_ = counters_.instruction_counter;
      ++counters_.instruction_counter;
    }
    for (auto const instr : block->instructions()) {
      for (auto const output : instr->outputs()) {
        if (output.is_virtual())
          ++counters_.output_counter;
      }
      instr->index_ = counters_.instruction_counter;
      ++counters_.instruction_counter;
    }
  }
  is_index_valid_ = true;
  return counters_;
}

void Editor::Append(Instruction* new_instruction) {
  DCHECK(!new_instruction->basic_block_);
  DCHECK(!new_instruction->id_);
  DCHECK(basic_block_);
  DidInsertInstruction();
  new_instruction->id_ = factory()->NextInstructionId();
  new_instruction->basic_block_ = basic_block_;
  auto const last = basic_block_->last_instruction();
  if (last && last->IsTerminator()) {
    basic_block_->instructions_.InsertBefore(new_instruction, last);
    return;
  }
  basic_block_->instructions_.AppendNode(new_instruction);
  AddEdgesFrom(new_instruction);
  DidChangeControlFlow();
}

const DominatorTree<Function>& Editor::BuildDominatorTree() const {
  if (dominator_tree_)
    return *dominator_tree_;
  dominator_tree_ =
      std::move(DominatorTreeBuilder<Function, ForwardFlowGraph<Function>>(
                    function()).Build());
  return *dominator_tree_;
}

const DominatorTree<Function>& Editor::BuildPostDominatorTree() const {
  if (post_dominator_tree_)
    return *post_dominator_tree_;
  post_dominator_tree_ =
      std::move(DominatorTreeBuilder<Function, BackwardFlowGraph<Function>>(
                    function()).Build());
  return *post_dominator_tree_;
}

void Editor::BulkRemoveInstructions(WorkList<Instruction>* instructions) {
  DCHECK(!basic_block_);
  if (instructions->empty())
    return;
#ifndef NDEBUG
  WorkList<BasicBlock> changed_blocks;
#endif
  while (!instructions->empty()) {
    auto const instr = instructions->Pop();
    DCHECK(!instr->IsTerminator())
        << "BulkRemove can't remove terminator: " << *instr;
#ifndef NDEBUG
    auto const block = instr->basic_block_;
    if (!changed_blocks.Contains(block))
      changed_blocks.Push(block);
#endif
    RemoveInternal(instr);
  }
  DidRemoveInstruction();
#ifndef NDEBUG
  while (!changed_blocks.empty()) {
    auto const block = changed_blocks.Pop();
    DCHECK(Validate(block)) << *this;
  }
#endif
}

bool Editor::Commit() {
  DCHECK(basic_block_);
#ifdef NDEBUG
  basic_block_ = nullptr;
  return true;
#else
  auto const is_valid = Validate(basic_block_);
  basic_block_ = nullptr;
  return is_valid;
#endif
}

void Editor::DidChangeControlFlow() {
  is_index_valid_ = false;
  dominator_tree_.reset();
  post_dominator_tree_.reset();
  liveness_data_.reset();
  pre_order_list_.reset();
  post_order_list_.reset();
  reverse_pre_order_list_.reset();
  reverse_post_order_list_.reset();
}

void Editor::DidInsertInstruction() {
  is_index_valid_ = false;
}

void Editor::DidRemoveInstruction() {
  is_index_valid_ = false;
}

void Editor::Edit(BasicBlock* basic_block) {
  DCHECK(!basic_block_) << basic_block;
  DCHECK_EQ(function(), basic_block->function()) << basic_block;
  basic_block_ = basic_block;
  if (basic_block_->instructions().empty())
    return;
  DCHECK(Validate(basic_block_)) << *this;
}

void Editor::EditNewBasicBlock() {
  Edit(NewBasicBlock(exit_block()));
}

void Editor::InsertAfter(Instruction* new_instruction,
                         Instruction* ref_instruction) {
  DCHECK(ref_instruction);
  InsertBefore(new_instruction, ref_instruction->next());
}

void Editor::InsertBefore(Instruction* new_instruction,
                          Instruction* ref_instruction) {
  if (!ref_instruction) {
    Append(new_instruction);
    return;
  }
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, ref_instruction->basic_block());
  DCHECK(!new_instruction->basic_block_);
  DCHECK(!new_instruction->id_);
  basic_block_->instructions_.InsertBefore(new_instruction, ref_instruction);
  new_instruction->id_ = factory()->NextInstructionId();
  new_instruction->basic_block_ = basic_block_;
  DidInsertInstruction();
}

Value Editor::InsertCopyBefore(Value output,
                               Value input,
                               Instruction* ref_instruction) {
  DCHECK(output.is_output()) << output;
  DCHECK(basic_block_) << output;
  if (ref_instruction) {
    auto const previous = ref_instruction->previous();
    if (previous && previous->is<CopyInstruction>() &&
        previous->output(0) == input && previous->input(0) == output) {
      // Avoid to emit useless copy
      //   copy %input = %output
      //   copy %output = %input
      // TODO(eval1749) We believe LIR transforms don't insert useless copy.
      NOTREACHED() << "useless copy instruction: " << *ref_instruction;
    }
  }
  InsertBefore(factory()->NewCopyInstruction(output, input), ref_instruction);
  return output;
}

BasicBlock* Editor::NewBasicBlock(BasicBlock* reference) {
  DCHECK(reference);
  DCHECK_EQ(function(), reference->function()) << reference;
  auto const new_block = factory()->NewBasicBlock();
  new_block->function_ = function();
  new_block->id_ = factory()->NextBasicBlockId();
  // We keep exit block at end of basic block list.
  graph_editor_.InsertNode(new_block, reference);
  return new_block;
}

PhiInstruction* Editor::NewPhi(Value output) {
  DCHECK(basic_block_) << output;
  auto const phi_instruction = factory()->NewPhiInstruction(output);
  basic_block_->phi_instructions_.AppendNode(phi_instruction);
  phi_instruction->basic_block_ = basic_block_;
  phi_instruction->id_ = factory()->NextInstructionId();
  return phi_instruction->as<PhiInstruction>();
}

const OrderedBlockList& Editor::PreOrderList() const {
  if (!pre_order_list_) {
    pre_order_list_.reset(
        new OrderedBlockList(Function::Sorter::SortByPreOrder(function())));
  }
  return *pre_order_list_;
}

const OrderedBlockList& Editor::PostOrderList() const {
  if (!post_order_list_) {
    post_order_list_.reset(
        new OrderedBlockList(Function::Sorter::SortByPostOrder(function())));
  }
  return *post_order_list_;
}

void Editor::Remove(Instruction* old_instruction) {
  DCHECK(basic_block_) << old_instruction;
  DCHECK_EQ(basic_block_, old_instruction->basic_block_) << old_instruction;
  DidRemoveInstruction();
  if (old_instruction->IsTerminator()) {
    RemoveEdgesFrom(old_instruction);
    DidChangeControlFlow();
  }
  RemoveInternal(old_instruction);
}

// Remove edges between |instruction|'s block and old successors.
void Editor::RemoveEdgesFrom(Instruction* instruction) {
  if (!instruction->IsTerminator())
    return;
  auto const block = instruction->basic_block();
  for (auto successor : instruction->block_operands()) {
    graph_editor_.RemoveEdge(block, successor);
  }
}

void Editor::RemoveInternal(Instruction* old_instruction) {
  old_instruction->basic_block_->instructions_.RemoveNode(old_instruction);
  old_instruction->id_ = 0;
  old_instruction->basic_block_ = nullptr;
}

void Editor::Replace(Instruction* new_instruction,
                     Instruction* old_instruction) {
  DCHECK(!new_instruction->IsTerminator())
      << "Please use Editor::SetTerminator() to replace terminator"
      << new_instruction << " " << old_instruction;
  DCHECK(basic_block_) << new_instruction << " " << old_instruction;
  DCHECK_EQ(basic_block_, old_instruction->basic_block_) << old_instruction;
  DCHECK(!new_instruction->basic_block()) << new_instruction;
  basic_block_->instructions_.ReplaceNode(new_instruction, old_instruction);
  new_instruction->id_ = old_instruction->id_;
  new_instruction->basic_block_ = basic_block_;
  old_instruction->id_ = 0;
  old_instruction->basic_block_ = nullptr;
  DidRemoveInstruction();
  DidInsertInstruction();
}

const OrderedBlockList& Editor::ReversePreOrderList() const {
  if (!reverse_pre_order_list_) {
    reverse_pre_order_list_.reset(new OrderedBlockList(
        Function::Sorter::SortByReversePreOrder(function())));
  }
  return *pre_order_list_;
}

const OrderedBlockList& Editor::ReversePostOrderList() const {
  if (!reverse_post_order_list_) {
    reverse_post_order_list_.reset(new OrderedBlockList(
        Function::Sorter::SortByReversePostOrder(function())));
  }
  return *reverse_post_order_list_;
}

void Editor::SetBlockOperand(Instruction* instruction,
                             int index,
                             BasicBlock* new_block) {
  DCHECK(basic_block_) << instruction;
  DCHECK_EQ(basic_block_->last_instruction(), instruction);
  RemoveEdgesFrom(instruction);
  instruction->SetBlockOperand(index, new_block);
  AddEdgesFrom(instruction);
}

void Editor::SetBranch(Value condition,
                       BasicBlock* true_block,
                       BasicBlock* false_block) {
  DCHECK(basic_block_) << condition << " " << true_block << " " << false_block;
  DCHECK(false_block->id()) << false_block;
  DCHECK(true_block->id()) << true_block;
  if (auto const last =
          basic_block_->last_instruction()->as<BranchInstruction>()) {
    SetInput(last, 0, condition);
    RemoveEdgesFrom(last);
    last->SetBlockOperand(0, true_block);
    last->SetBlockOperand(1, false_block);
    AddEdgesFrom(last);
    return;
  }
  SetTerminator(
      factory()->NewBranchInstruction(condition, true_block, false_block));
}

void Editor::SetInput(Instruction* instruction, int index, Value new_value) {
  DCHECK(basic_block_) << instruction;
  DCHECK_EQ(basic_block_, instruction->basic_block()) << instruction;
  instruction->SetInput(index, new_value);
}

void Editor::SetJump(BasicBlock* target_block) {
  DCHECK(basic_block_) << target_block;
  if (auto const last =
          basic_block_->last_instruction()->as<JumpInstruction>()) {
    RemoveEdgesFrom(last);
    last->SetBlockOperand(0, target_block);
    AddEdgesFrom(last);
    return;
  }
  SetTerminator(factory()->NewJumpInstruction(target_block));
}

void Editor::SetPhiInput(PhiInstruction* phi,
                         BasicBlock* block,
                         Value new_value) {
  DCHECK_EQ(basic_block_, phi->basic_block()) << phi;
  DCHECK(basic_block_) << phi;
  if (auto const present = phi->FindPhiInputFor(block)) {
    present->value_ = new_value;
    return;
  }
  auto const new_input = new (factory()->zone()) PhiInput(block, new_value);
  phi->phi_inputs_.AppendNode(new_input);
}

void Editor::SetOutput(Instruction* instruction, int index, Value new_value) {
  DCHECK(basic_block_) << instruction;
  DCHECK_EQ(basic_block_, instruction->basic_block()) << instruction;
  instruction->SetOutput(index, new_value);
}

void Editor::SetReturn() {
  DCHECK(basic_block_);
  if (auto const last = basic_block_->last_instruction()->as<RetInstruction>())
    return;
  SetTerminator(factory()->NewRetInstruction(exit_block()));
}

void Editor::SetTerminator(Instruction* instr) {
  DCHECK(basic_block_) << instr;
  DCHECK(!instr->basic_block_) << instr;
  DCHECK(instr->IsTerminator()) << instr;
  auto const last = basic_block_->last_instruction();
  if (last && last->IsTerminator())
    Remove(last);
  Append(instr);
}

// Replaces phi input for |old_block| to |new_block|.
void Editor::ReplacePhiInputs(BasicBlock* new_block, BasicBlock* old_block) {
  DCHECK(basic_block_) << new_block << " " << old_block;
  DCHECK_NE(new_block, old_block);
  for (auto const phi : basic_block_->phi_instructions())
    phi->FindPhiInputFor(old_block)->basic_block_ = new_block;
}

bool Editor::Validate() {
  Validator validator(this);
  return validator.Validate(function_);
}

bool Editor::Validate(BasicBlock* block) {
  Validator validator(this);
  return validator.Validate(block);
}

bool Editor::Validate(Function* function) {
  Validator validator(this);
  return validator.Validate(function);
}

std::ostream& operator<<(std::ostream& ostream, const Editor& editor) {
  TextFormatter formatter(editor.factory()->literals(), &ostream);
  formatter.FormatFunction(editor.function());
  if (editor.factory()->errors().empty())
    return ostream;
  return ostream << std::endl
                 << "Errors:" << std::endl
                 << editor.factory()->errors() << std::endl;
}

}  // namespace lir
}  // namespace elang
