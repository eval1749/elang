// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EDITOR_H_
#define ELANG_LIR_EDITOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/graphs/graph_editor.h"
#include "elang/lir/editor.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class BasicBlock;
enum class ErrorCode;
class ErrorData;
class Factory;
class Function;
class Instruction;
class PhiInstruction;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class ELANG_LIR_EXPORT Editor final {
 public:
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
  const std::vector<ErrorData*>& errors() { return errors_; }
  Factory* factory() const { return factory_; }
  Function* function() const { return function_; }

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
  void SetBranch(Value condition,
                 BasicBlock* true_block,
                 BasicBlock* false_block);
  void SetJump(BasicBlock* target_block);
  void SetReturn();
  void SetTerminator(Instruction* instruction);

  // Returns new basic block inserted before |reference|.
  BasicBlock* NewBasicBlock(BasicBlock* reference);

  // Instruction editing
  void Append(Instruction* new_instruction);
  void InsertBefore(Instruction* new_instruction, Instruction* ref_instruction);
  void Remove(Instruction* old_instruction);
  void SetInput(Instruction* instruction, int index, Value new_value);
  void SetOutput(Instruction* instruction, int index, Value new_value);

  // Phi instruction
  PhiInstruction* NewPhi(Value output);
  void SetPhiInput(PhiInstruction* phi_instruction,
                   BasicBlock* basic_block,
                   Value value);

  // Expose |Validate()| for testing in release build.
  bool Validate();

 private:
  void ResetSuccessors(Instruction* instruction);
  void SetSuccessors(Instruction* instruction);
  bool Validate(BasicBlock* basic_block);
  bool Validate(Function* function);

  // A basic block being edited, or null if not editing.
  BasicBlock* basic_block_;
  // List of errors found by validator
  std::vector<ErrorData*> errors_;
  // The factory
  Factory* const factory_;
  // A function being edited.
  Function* const function_;

  GraphEditor<Function, BasicBlock> graph_editor_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EDITOR_H_
