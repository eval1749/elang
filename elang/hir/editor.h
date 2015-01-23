// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_EDITOR_H_
#define ELANG_HIR_EDITOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_user.h"
#include "elang/hir/editor.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

class BasicBlock;
enum class ErrorCode;
class ErrorData;
class Factory;
class Function;
class Instruction;
class Value;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class ELANG_HIR_EXPORT Editor final : public ZoneUser {
 public:
  class ELANG_HIR_EXPORT ScopedEdit final {
   public:
    explicit ScopedEdit(Editor* editor) : editor_(editor) {}
    ~ScopedEdit() { editor_->Commit(); }

   private:
    Editor* const editor_;

    DISALLOW_COPY_AND_ASSIGN(ScopedEdit);
  };

  Editor(Factory* factory, Function* function);
  ~Editor();

  const std::vector<ErrorData*> errors() const { return errors_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  Factory* factory() const { return factory_; }

  // Validation errors
  void Error(ErrorCode, Value* value);
  void Error(ErrorCode, Value* value, Value* message);
  void Error(ErrorCode, Value* value, const std::vector<Value*> params);

  // Commit changes
  bool Commit();

  // Basic block editing
  void Edit(BasicBlock* basic_block);
  BasicBlock* NewBasicBlock();

  // Instruction editing
  void Append(Instruction* new_instruction);
  void InsertBefore(Instruction* new_instruction, Instruction* ref_instruction);
  void RemoveInstruction(Instruction* old_instruction);

  // Operand manupulation
  void SetInput(Instruction* instruction, int index, Value* new_value);

  // Set terminator instruction
  void SetBranch(Value* condition,
                 BasicBlock* then_block,
                 BasicBlock* else_block);
  // Set unconditional branch
  void SetBranch(BasicBlock* target_block);
  void SetReturn(Value* new_value);
  void SetTerminator(Instruction* terminator);

  // Validation
  bool Validate(BasicBlock* basic_block);
  bool Validate(Function* function);

 private:
  void InitializeFunctionIfNeeded();

  std::vector<BasicBlock*> basic_blocks_;
  std::vector<ErrorData*> errors_;
  Factory* const factory_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_EDITOR_H_
