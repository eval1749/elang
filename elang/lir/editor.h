// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EDITOR_H_
#define ELANG_LIR_EDITOR_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "elang/base/graphs/graph_editor.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/literals_forward.h"

namespace elang {

template <typename Node, typename Variable>
class LivenessCollection;

template <typename Graph>
class DominatorTree;

namespace lir {

enum class ErrorCode;
class ErrorData;
class Factory;
class Instruction;
class PhiInstruction;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class ELANG_LIR_EXPORT Editor final {
 public:
  typedef LivenessCollection<BasicBlock*, Value> LivenessData;

  // Counters
  struct Counters {
    int block_counter;
    int instruction_counter;
    int output_counter;
  };

  // ScopedEdit
  class ELANG_LIR_EXPORT ScopedEdit final {
   public:
    explicit ScopedEdit(Editor* editor) : editor_(editor) {}
    ~ScopedEdit() { editor_->Commit(); }

   private:
    Editor* const editor_;

    DISALLOW_COPY_AND_ASSIGN(ScopedEdit);
  };

  Editor(Factory* factory, Function* function);
  ~Editor();

  BasicBlock* basic_block() const { return basic_block_; }
  BasicBlock* entry_block() const;
  const std::vector<ErrorData*>& errors() { return errors_; }
  BasicBlock* exit_block() const;
  Factory* factory() const { return factory_; }
  Function* function() const { return function_; }

  // Analysis
  const LivenessData& AnalyzeLiveness() const;
  Counters AssignIndex();

  // Dominator tree
  const DominatorTree<Function>& BuildDominatorTree() const;
  const DominatorTree<Function>& BuildPostDominatorTree() const;

  // Validation errors
  void AddError(ErrorCode error_code,
                Value value,
                const std::vector<Value> details);
  void Error(ErrorCode error_code, Value value);
  void Error(ErrorCode error_code, Value value, Value detail);
  void Error(ErrorCode error_code, Value value, Value detail1, Value detail2);

  // Basic block editing
  bool Commit();
  void Edit(BasicBlock* basic_block);
  void EditNewBasicBlock();

  // Remove critical edges to blocks having 'phi' instructions. Editor must not
  // be editing block.
  void RemoveCriticalEdges();

  // Set |index|'s block operand by |new_block|. This function doesn't upddate
  // `phi` instruction. You may need to update `phi` instruction by
  // |ReplacePhiInput(new_block, old_block)|.
  void SetBlockOperand(Instruction* instruction,
                       int index,
                       BasicBlock* new_block);
  void SetBranch(Value condition,
                 BasicBlock* true_block,
                 BasicBlock* false_block);
  void SetJump(BasicBlock* target_block);
  void SetReturn();
  void SetTerminator(Instruction* instruction);

  // Basic block ordered list.
  const OrderedBlockList& PreOrderList() const;
  const OrderedBlockList& PostOrderList() const;
  const OrderedBlockList& ReversePreOrderList() const;
  const OrderedBlockList& ReversePostOrderList() const;

  // Returns new basic block inserted before |reference|.
  BasicBlock* NewBasicBlock(BasicBlock* reference);

  // Instruction editing
  void Append(Instruction* new_instruction);
  void InsertAfter(Instruction* new_instruction, Instruction* ref_instruction);
  void InsertBefore(Instruction* new_instruction, Instruction* ref_instruction);
  void Remove(Instruction* old_instruction);
  void Replace(Instruction* new_instruction, Instruction* old_instruction);
  void SetInput(Instruction* instruction, int index, Value new_value);
  void SetOutput(Instruction* instruction, int index, Value new_value);

  // Emit instructions
  Value InsertCopyBefore(Value output,
                         Value input,
                         Instruction* ref_instruction);

  // Phi instruction
  PhiInstruction* NewPhi(Value output);
  void SetPhiInput(PhiInstruction* phi_instruction,
                   BasicBlock* basic_block,
                   Value value);
  // Replaces phi input for |old_block| to |new_block|.
  void ReplacePhiInputs(BasicBlock* new_block, BasicBlock* old_block);

  // Expose |Validate()| for testing in release build.
  bool Validate(BasicBlock* basic_block);
  bool Validate();

 private:
  void AddEdgesFrom(Instruction* instruction);
  void DidChangeControlFlow();
  void RemoveEdgesFrom(Instruction* instruction);
  bool Validate(Function* function);

  // A basic block being edited, or null if not editing.
  BasicBlock* basic_block_;
  // List of errors found by validator
  std::vector<ErrorData*> errors_;
  // The factory
  Factory* const factory_;
  // A function being edited.
  Function* const function_;

  // Cached liveness
  mutable std::unique_ptr<DominatorTree<Function>> dominator_tree_;
  mutable std::unique_ptr<DominatorTree<Function>> post_dominator_tree_;

  // Cached basic block lists.
  mutable std::unique_ptr<OrderedBlockList> pre_order_list_;
  mutable std::unique_ptr<OrderedBlockList> post_order_list_;
  mutable std::unique_ptr<OrderedBlockList> reverse_pre_order_list_;
  mutable std::unique_ptr<OrderedBlockList> reverse_post_order_list_;

  // Cached liveness
  mutable std::unique_ptr<LivenessData> liveness_data_;

  GraphEditor<Function, BasicBlock> graph_editor_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EDITOR_H_
