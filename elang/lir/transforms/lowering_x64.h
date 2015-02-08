// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_LOWERING_X64_H_
#define ELANG_LIR_TRANSFORMS_LOWERING_X64_H_

#include "elang/lir/editor_owner.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/pass.h"

namespace elang {
namespace lir {

class Factory;
class Function;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// X64LoweringPass does:
//  - Transforms three operands instruction to two operands.
//  - Transforms 'mul' to use 'RAX'
//  - Transforms 'div' to use 'RAX'/'RDX'
//
class ELANG_LIR_EXPORT X64LoweringPass final : public FunctionPass,
                                               public EditorOwner,
                                               public InstructionVisitor {
 public:
  X64LoweringPass(Factory* factory, Function* function);
  ~X64LoweringPass() final;

 private:
  // Pass
  base::StringPiece name() const final;

  // Function
  void RunOnFunction() final;

  // Support functions
  void RewriteToTwoOperands(Instruction* instr);

  // InstructionVisitor
  void VisitAdd(AddInstruction* instr) final;
  void VisitBitAnd(BitAndInstruction* instr) final;
  void VisitBitOr(BitOrInstruction* instr) final;
  void VisitBitXor(BitXorInstruction* instr) final;
  void VisitSub(SubInstruction* instr) final;

  DISALLOW_COPY_AND_ASSIGN(X64LoweringPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_LOWERING_X64_H_
