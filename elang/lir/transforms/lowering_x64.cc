// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/lowering_x64.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target_x64.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

X64LoweringPass::X64LoweringPass(Factory* factory, Function* function)
    : FunctionPass(factory, function), EditorOwner(factory, function) {
}

X64LoweringPass::~X64LoweringPass() {
}

base::StringPiece X64LoweringPass::name() const {
  return "lowering_x64";
}

// Rewrite three operands instruction to two operands instruction.
//   add %a = %b, %c
//   =>
//   assign %1 = %b
//   add %2 = %1, %c
//   copy %a = %2
void X64LoweringPass::RewriteToTwoOperands(Instruction* instr) {
  // TODO(eval1749) If target supports VEX instruction, we don't need to rewrite
  // floating operation to two operands.
  auto const output = instr->output(0);
  auto const assign_instr =
      NewAssignInstruction(NewRegister(output), instr->input(0));
  editor()->InsertBefore(assign_instr, instr);
  auto const new_output = NewRegister(output);
  editor()->SetOutput(instr, 0, new_output);
  editor()->SetInput(instr, 0, assign_instr->output(0));
  editor()->InsertCopyBefore(output, new_output, instr->next());
}

void X64LoweringPass::RunOnFunction() {
  for (auto const block : function()->basic_blocks()) {
    editor()->Edit(block);
    auto instr = block->first_instruction();
    while (instr) {
      auto const next_instr = instr->next();
      instr->Accept(this);
      instr = next_instr;
    }
    editor()->Commit();
  }
}

// InstructionVisitor

void X64LoweringPass::VisitAdd(AddInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void X64LoweringPass::VisitBitAnd(BitAndInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void X64LoweringPass::VisitBitOr(BitOrInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void X64LoweringPass::VisitBitXor(BitXorInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void X64LoweringPass::VisitSub(SubInstruction* instr) {
  RewriteToTwoOperands(instr);
}

}  // namespace lir
}  // namespace elang
