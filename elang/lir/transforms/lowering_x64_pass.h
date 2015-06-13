// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_LOWERING_X64_PASS_H_
#define ELANG_LIR_TRANSFORMS_LOWERING_X64_PASS_H_

#include "elang/lir/instructions_forward.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/pass.h"

namespace elang {
namespace lir {

struct Value;

//////////////////////////////////////////////////////////////////////
//
// LoweringX64Pass does:
//  - Transforms three operands instruction to two operands.
//  - Transforms 'mul' to use 'RAX'
//  - Transforms 'div' to use 'RAX'/'RDX'
//
class ELANG_LIR_EXPORT LoweringX64Pass final : public FunctionPass,
                                               public InstructionVisitor {
 public:
  LoweringX64Pass(base::StringPiece name, Editor* editor);
  ~LoweringX64Pass() final;

 private:
  // FunctionPass
  void RunOnFunction() final;

  // Support functions
  Value GetRAX(Value type);
  Value GetRDX(Value type);
  void RewriteShiftInstruciton(Instruction* instr);
  void RewriteToTwoOperands(Instruction* instr);

  // InstructionVisitor
  void VisitAdd(AddInstruction* instr) final;
  void VisitBitAnd(BitAndInstruction* instr) final;
  void VisitBitOr(BitOrInstruction* instr) final;
  void VisitBitXor(BitXorInstruction* instr) final;
  void VisitDiv(DivInstruction* instr) final;
  void VisitFloatAdd(FloatAddInstruction* instr) final;
  void VisitFloatDiv(FloatDivInstruction* instr) final;
  void VisitFloatMod(FloatModInstruction* instr) final;
  void VisitFloatMul(FloatMulInstruction* instr) final;
  void VisitFloatSub(FloatSubInstruction* instr) final;
  void VisitMul(MulInstruction* instr) final;
  void VisitShl(ShlInstruction* instr) final;
  void VisitShr(ShrInstruction* instr) final;
  void VisitSub(SubInstruction* instr) final;

  DISALLOW_COPY_AND_ASSIGN(LoweringX64Pass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_LOWERING_X64_PASS_H_
