// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_VALIDATOR_H_
#define ELANG_LIR_VALIDATOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class BasicBlock;
enum class ErrorCode;
class ErrorData;
class Editor;
class Function;
class Instruction;
class Literal;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// Validator
//
class ELANG_LIR_EXPORT Validator final : public InstructionVisitor {
 public:
  explicit Validator(Editor* editor);
  ~Validator();

  bool Validate(BasicBlock* basic_block);
  bool Validate(Function* function);
  bool Validate(Instruction* instruction);

 private:
  Editor* editor() const { return editor_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  Function* function() const;

  // Validation errors
  void AddError(ErrorCode error_code,
                Value value,
                const std::vector<Value> details);
  Value AsValue(Instruction* instruction);

  // Helper functions for reporting error
  void Error(ErrorCode error_code, Instruction* instruction);
  void Error(ErrorCode error_code, Instruction* instruction, int detail);
  void Error(ErrorCode error_code, Instruction* instruction, Value detail);
  void Error(ErrorCode error_code, Value value);
  void Error(ErrorCode error_code, Value value, Value detail);
  void Error(ErrorCode error_code, Value value, Value detail1, Value detail2);

  Literal* GetLiteral(Value value);

  // InstructionVisitor
  void VisitBranch(BranchInstruction* instruction) final;
  void VisitCopy(CopyInstruction* instruction) final;
  void VisitRet(RetInstruction* instruction) final;

  Editor* const editor_;

  DISALLOW_COPY_AND_ASSIGN(Validator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_VALIDATOR_H_
