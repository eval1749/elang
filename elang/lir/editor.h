// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EDITOR_H_
#define ELANG_LIR_EDITOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/lir/editor.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class BasicBlock;
class Factory;
class Function;
class Instruction;
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
  Factory* factory() const { return factory_; }
  Function* function() const { return function_; }

  bool Commit();

  // Basic block editing
  void Edit(BasicBlock* basic_block);
  void EditNewBasicBlock();
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

  // Validation
  static bool Validate(BasicBlock* basic_block);
  static bool Validate(Function* function);

 private:
  void InitializeFunctionIfNeeded();

  BasicBlock* basic_block_;
  Factory* const factory_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EDITOR_H_
