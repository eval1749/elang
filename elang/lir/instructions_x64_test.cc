// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions_x64.h"
#include "elang/lir/target_x64.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirInstructionsTestX64
//
class LirInstructionsTestX64 : public testing::LirTestX64 {
 protected:
  LirInstructionsTestX64() = default;

  static std::string ToString(const Instruction& instr);

 private:
  DISALLOW_COPY_AND_ASSIGN(LirInstructionsTestX64);
};

std::string LirInstructionsTestX64::ToString(const Instruction& instr) {
  std::ostringstream ostream;
  ostream << instr;
  return ostream.str();
}

// Test cases...

TEST_F(LirInstructionsTestX64, BranchInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  auto const true_block = editor.NewBasicBlock(function->exit_block());
  auto const false_block = editor.NewBasicBlock(function->exit_block());
  auto const merge_block = editor.NewBasicBlock(function->exit_block());

  editor.Edit(function->entry_block());
  editor.SetBranch(factory()->NewConditional(), true_block, false_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(true_block);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(false_block);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(merge_block);
  auto const phi = editor.NewPhi(factory()->NewRegister(Value::Int32Type()));
  editor.SetPhiInput(phi, true_block, Value::SmallInt32(42));
  editor.SetPhiInput(phi, false_block, Value::SmallInt32(39));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3, block4}\n"
      "  entry\n"
      "  br %b2, block3, block4\n"
      "block3:\n"
      "  // In: {block1}\n"
      "  // Out: {block5}\n"
      "  jmp block5\n"
      "block4:\n"
      "  // In: {block1}\n"
      "  // Out: {block5}\n"
      "  jmp block5\n"
      "block5:\n"
      "  // In: {block3, block4}\n"
      "  // Out: {block2}\n"
      "  phi %r1 = block3 42, block4 39\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block5}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(LirInstructionsTestX64, CopyInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewCopyInstruction(Target::RegisterOf(isa::RAX),
                                              NewIntPtrRegister()));
  EXPECT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  mov RAX = %r1l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(LirInstructionsTestX64, LoadInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const destination = NewIntPtrRegister();
  editor.Append(factory()->NewLoadInstruction(
      destination, Value::Parameter(destination, 4),
      Value::Parameter(destination, 4), Value::SmallInt32(42)));
  EXPECT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  load %r1l = %param[4], %param[4], 42\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(LirInstructionsTestX64, UIntMulX64Instruction) {
  auto const eax = Target::RegisterOf(isa::EAX);
  auto const edx = Target::RegisterOf(isa::EDX);
  auto const instr = factory()->NewUIntMulX64Instruction(eax, edx, eax, edx);
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(2, instr->outputs().size());
  EXPECT_EQ("--:0:x64.umul EAX, EDX = EAX, EDX", ToString(*instr));
}

}  // namespace lir
}  // namespace elang
