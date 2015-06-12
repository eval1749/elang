// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_VALIDATOR_H_
#define ELANG_LIR_VALIDATOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/lir/error_reporter.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class BasicBlock;
enum class ErrorCode;
class ErrorData;
class Editor;
class Factory;
class Function;
class Instruction;
class Literal;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// Validator
//
class ELANG_LIR_EXPORT Validator final : public ErrorReporter,
                                         public InstructionVisitor {
 public:
  explicit Validator(Editor* editor);
  ~Validator();

  bool Validate(BasicBlock* basic_block);
  bool Validate(Function* function);
  bool Validate(Instruction* instruction);

 private:
  const Editor* editor() const { return editor_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  Factory* Validator::factory() const;
  Function* function() const;

  Literal* GetLiteral(Value value);

  void ValidateArithmeticInstruction(Instruction* instr);

  // InstructionVisitor
  void VisitAdd(AddInstruction* instruction) final;
  void VisitBitAnd(BitAndInstruction* instruction) final;
  void VisitBitOr(BitOrInstruction* instruction) final;
  void VisitBitXor(BitXorInstruction* instruction) final;
  void VisitBranch(BranchInstruction* instruction) final;
  void VisitCmp(CmpInstruction* instruction) final;
  void VisitCopy(CopyInstruction* instruction) final;
  void VisitDiv(DivInstruction* instruction) final;
  void VisitExtend(ExtendInstruction* instruction) final;
  void VisitFloatCmp(FloatCmpInstruction* instruction) final;
  void VisitLoad(LoadInstruction* instruction) final;
  void VisitMul(MulInstruction* instruction) final;
  void VisitPhi(PhiInstruction* instruction) final;
  void VisitRet(RetInstruction* instruction) final;
  void VisitSignedConvert(SignedConvertInstruction* instruction) final;
  void VisitSignExtend(SignExtendInstruction* instruction) final;
  void VisitStore(StoreInstruction* instruction) final;
  void VisitSub(SubInstruction* instruction) final;
  void VisitTruncate(TruncateInstruction* instruction) final;
  void VisitUse(UseInstruction* instruction) final;
  void VisitUnsignedConvert(UnsignedConvertInstruction* instruction) final;
  void VisitZeroExtend(ZeroExtendInstruction* instruction) final;

  Editor* const editor_;

  DISALLOW_COPY_AND_ASSIGN(Validator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_VALIDATOR_H_
