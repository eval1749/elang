// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_EDITOR_H_
#define ELANG_HIR_EDITOR_H_

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "elang/hir/error_reporter.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/factory_user.h"

namespace elang {
namespace hir {

class BasicBlock;
class DominatorTree;
enum class ErrorCode;
class ErrorData;
class Factory;
class Function;
class Instruction;
class PhiInstruction;
class Thing;
class Type;
class Value;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class ELANG_HIR_EXPORT Editor final : public ErrorReporter, public FactoryUser {
 public:
  class ELANG_HIR_EXPORT ScopedEdit final {
   public:
    explicit ScopedEdit(Editor* editor, BasicBlock* basic_block);
    ~ScopedEdit();

   private:
    Editor* const editor_;

    DISALLOW_COPY_AND_ASSIGN(ScopedEdit);
  };

  Editor(Factory* factory, Function* function);
  ~Editor();

  BasicBlock* basic_block() const { return basic_block_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  Function* function() const { return function_; }

  // Returns dominator tree for current editing function if available.
  DominatorTree* maybe_dominator_tree() const;

  ////////////////////////////////////////////////////////////
  //
  // Analysis
  //
  DominatorTree* ComputeDominatorTree();

  ////////////////////////////////////////////////////////////
  //
  // Operations on |BasicBlock|
  //
  bool Commit();

  // Edit a basic block as result of |SplitBefore()|, since |SplitBefore()|
  // returns a basic block without terminator.
  void Continue(BasicBlock* basic_block);

  // Edit |basic_block|
  void Edit(BasicBlock* basic_block);

  // Returns new basic block inserted before |reference| block and starts
  // editing.
  BasicBlock* EditNewBasicBlock(BasicBlock* reference);

  // Returns new basic block inserted before exit block and starts editing.
  BasicBlock* EditNewBasicBlock();

  // Returns new basic block inserted before |reference| block.
  BasicBlock* NewBasicBlock(BasicBlock* reference);

  // Returns new basic block inserted after basic block containing
  // |reference| instruction and moves instructions starting from |reference|
  // to the last instructions.
  BasicBlock* SplitBefore(Instruction* reference);

  ////////////////////////////////////////////////////////////
  //
  // Operations on |Instruction|
  //
  void Append(Instruction* new_instruction);
  void InsertBefore(Instruction* new_instruction, Instruction* ref_instruction);
  void RemoveInstruction(Instruction* old_instruction);

  // Operand manipulation
  // Replaces all uses of |old_value| by |new_value| and removes |old_value|.
  // |Editor| must be editing a basic block of |old_value|.
  void ReplaceAll(Value* new_value, Instruction* old_value);

  // Set input operand at |index| of |instruction| to |new_value|.
  // |Editor| must be editing a basic block of |instruction|.
  void SetInput(Instruction* instruction, int index, Value* new_value);

  // Phi instruction
  PhiInstruction* NewPhi(Type* output_type);
  void SetPhiInput(PhiInstruction* phi_instruction,
                   BasicBlock* basic_block,
                   Value* value);

  // Set terminator instruction
  void SetBranch(Value* condition,
                 BasicBlock* then_block,
                 BasicBlock* else_block);
  // Set unconditional branch
  void SetBranch(BasicBlock* target_block);
  void SetReturn(Value* new_value);
  void SetTerminator(Instruction* terminator);
  void SetThrow(Value* new_value);
  void SetUnreachable();

  // Values
  Value* NewInt32(int32_t data);

  // Expose |Validate()| for testing in release build.
  bool Validate(BasicBlock* basic_block);
  bool Validate();

 private:
  void DidChangeControlFlow();
  void InitializeFunctionIfNeeded();
  void ResetInputs(Instruction* instruction);
  bool Validate(Function* function);

  BasicBlock* basic_block_;
  std::unique_ptr<DominatorTree> dominator_tree_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_EDITOR_H_
