// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_EDITOR_H_
#define ELANG_HIR_EDITOR_H_

#include <vector>

#include "base/basictypes.h"
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
class PhiInstruction;
class Thing;
class Type;
class Value;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class ELANG_HIR_EXPORT Editor final : public ZoneUser {
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

  const std::vector<ErrorData*> errors() const { return errors_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  Function* function() const { return function_; }
  Factory* factory() const { return factory_; }

  // Validation errors
  void Error(ErrorCode error_code, const Value* value);
  void Error(ErrorCode error_code, const Value* value, Thing* detail);
  void Error(ErrorCode error_code,
             const Value* value,
             const std::vector<Thing*>& details);
  void Error(ErrorCode error_code, const Instruction* instruction, int index);
  void Error(ErrorCode error_code,
             const Instruction* instruction,
             int index,
             Thing* detail);

  // Commit changes
  bool Commit();

  // Operations on |BasicBlock|
  void Continue(BasicBlock* basic_block);
  void Edit(BasicBlock* basic_block);
  BasicBlock* EditNewBasicBlock(BasicBlock* reference);
  BasicBlock* EditNewBasicBlock();
  BasicBlock* NewBasicBlock(BasicBlock* reference);
  // Returns new basic block inserted after basic block containing
  // |reference| instruction and moves instructions starting from |reference|
  // to the last instructions.
  BasicBlock* SplitBefore(Instruction* reference);

  // Instruction editing
  void Append(Instruction* new_instruction);
  void InsertBefore(Instruction* new_instruction, Instruction* ref_instruction);
  void RemoveInstruction(Instruction* old_instruction);

  // Operand manipulation
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

  // Values
  Value* NewInt32(int32_t data);

  // Validation
  bool Validate(BasicBlock* basic_block);
  bool Validate(Function* function);

 private:
  void InitializeFunctionIfNeeded();
  void ResetInputs(Instruction* instruction);

  BasicBlock* basic_block_;
  std::vector<ErrorData*> errors_;
  Factory* const factory_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_EDITOR_H_
