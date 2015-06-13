// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/lowering_x64_pass.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target_x64.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

LoweringX64Pass::LoweringX64Pass(base::StringPiece name, Editor* editor)
    : FunctionPass(name, editor) {
}

LoweringX64Pass::~LoweringX64Pass() {
}

Value LoweringX64Pass::GetRAX(Value type) {
  DCHECK(type.is_integer());
  return Target::RegisterOf(type.is_64bit() ? isa::RAX : isa::EAX);
}

Value LoweringX64Pass::GetRDX(Value type) {
  DCHECK(type.is_integer());
  return Target::RegisterOf(type.is_64bit() ? isa::RDX : isa::EDX);
}

// Rewrite count operand to use |CL| register.
void LoweringX64Pass::RewriteShiftInstruciton(Instruction* instr) {
  RewriteToTwoOperands(instr);
  auto const count_input = instr->input(1);
  if (!count_input.is_register())
    return;
  auto const count_register =
      Target::RegisterOf(count_input.is_64bit() ? isa::RCX : isa::ECX);
  editor()->InsertCopyBefore(count_register, count_input, instr);
  editor()->SetInput(instr, 1, count_register);
}

// Rewrite three operands instruction to two operands instruction.
//   add %a = %b, %c
//   =>
//   copy %1 = %b
//   add %2 = %1, %c
//   copy %a = %2
void LoweringX64Pass::RewriteToTwoOperands(Instruction* instr) {
  // TODO(eval1749) If target supports VEX instruction, we don't need to rewrite
  // floating operation to two operands.
  auto const output = instr->output(0);
  if (!instr->input(0).is_virtual()) {
    auto const new_input = NewRegister(output);
    editor()->InsertBefore(NewLiteralInstruction(new_input, instr->input(0)),
                           instr);
    editor()->SetInput(instr, 0, new_input);
  }
  auto const new_temp = NewRegister(output);
  editor()->InsertCopyBefore(new_temp, instr->input(0), instr);
  auto const new_output = NewRegister(output);
  editor()->SetOutput(instr, 0, new_output);
  editor()->SetInput(instr, 0, new_temp);
  editor()->InsertCopyBefore(output, new_output, instr->next());
}

void LoweringX64Pass::RunOnFunction() {
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

void LoweringX64Pass::VisitAdd(AddInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitBitAnd(BitAndInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitBitOr(BitOrInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitBitXor(BitXorInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitFloatAdd(FloatAddInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitFloatDiv(FloatDivInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitFloatMod(FloatModInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitFloatMul(FloatMulInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitFloatSub(FloatSubInstruction* instr) {
  RewriteToTwoOperands(instr);
}

//   div %a = %b, %c
//   =>
//   copy RAX = %b
//   sign_x64 RDX = RAX
//   div RAX, RDX = RDX, RAX, %c
//   copy %a = RAX
void LoweringX64Pass::VisitIntDiv(IntDivInstruction* instr) {
  auto const output = instr->output(0);
  auto const input =
      editor()->InsertCopyBefore(GetRAX(output), instr->input(0), instr);
  auto const sign_instr =
      factory()->NewIntSignX64Instruction(GetRDX(output), GetRAX(output));
  editor()->InsertBefore(sign_instr, instr);
  auto const div_instr = factory()->NewIntDivX64Instruction(
      GetRAX(output), GetRDX(output), sign_instr->output(0), input,
      instr->input(1));
  editor()->InsertBefore(div_instr, instr);
  editor()->Replace(NewCopyInstruction(output, div_instr->output(0)), instr);
}

void LoweringX64Pass::VisitIntMul(IntMulInstruction* instr) {
  RewriteToTwoOperands(instr);
}

void LoweringX64Pass::VisitShl(ShlInstruction* instr) {
  RewriteShiftInstruciton(instr);
}

void LoweringX64Pass::VisitShr(ShrInstruction* instr) {
  RewriteShiftInstruciton(instr);
}

void LoweringX64Pass::VisitSub(SubInstruction* instr) {
  RewriteToTwoOperands(instr);
}

//   udiv %a = %b, %c
//   =>
//   copy RAX = %b
//   xor RDX = RDX, RDX
//   udiv_x64 RAX, RDX = RDX, RAX, %c
//   copy %a = RAX
void LoweringX64Pass::VisitUIntDiv(UIntDivInstruction* instr) {
  auto const output = instr->output(0);
  auto const input =
      editor()->InsertCopyBefore(GetRAX(output), instr->input(0), instr);
  auto const zero_instr =
      NewBitXorInstruction(GetRDX(output), GetRDX(output), GetRDX(output));
  editor()->InsertBefore(zero_instr, instr);
  auto const div_instr = factory()->NewUIntDivX64Instruction(
      GetRAX(output), GetRDX(output), zero_instr->output(0), input,
      instr->input(1));
  editor()->InsertBefore(div_instr, instr);
  editor()->Replace(NewCopyInstruction(output, div_instr->output(0)), instr);
}

}  // namespace lir
}  // namespace elang
