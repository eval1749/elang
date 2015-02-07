// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
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

 private:
  DISALLOW_COPY_AND_ASSIGN(LirInstructionsTestX64);
};

// Test cases...

TEST_F(LirInstructionsTestX64, BranchInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  auto const true_block = editor.NewBasicBlock(function->exit_block());
  auto const false_block = editor.NewBasicBlock(function->exit_block());
  auto const merge_block = editor.NewBasicBlock(function->exit_block());

  editor.Edit(function->entry_block());
  editor.SetBranch(factory()->NewCondition(), true_block, false_block);
  editor.Commit();

  editor.Edit(true_block);
  editor.SetJump(merge_block);
  editor.Commit();

  editor.Edit(false_block);
  editor.SetJump(merge_block);
  editor.Commit();

  editor.Edit(merge_block);
  auto const phi = editor.NewPhi(factory()->NewRegister());
  editor.SetPhiInput(phi, true_block, Target::GetRegister(isa::EAX));
  editor.SetPhiInput(phi, false_block, Target::GetRegister(isa::EBX));
  editor.SetReturn();
  editor.Commit();

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  br %b2, block3, block4\n"
      "block3:\n"
      "  jmp block5\n"
      "block4:\n"
      "  jmp block5\n"
      "block5:\n"
      "  phi %r1l = block3 EAX, block4 EBX\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirInstructionsTestX64, CopyInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewCopyInstruction(Target::GetRegister(isa::RAX),
                                              factory()->NewRegister()));
  editor.Commit();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  mov RAX = %r1l\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirInstructionsTestX64, FunctionEmpty) {
  auto const function = CreateFunctionEmptySample();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirInstructionsTestX64, FunctionSample1) {
  auto const function = CreateFunctionSample1();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  call \"Foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirInstructionsTestX64, JumpInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  auto const block = editor.NewBasicBlock(function->exit_block());
  editor.Edit(block);
  editor.SetReturn();
  editor.Commit();
  editor.Edit(function->entry_block());
  editor.SetJump(block);
  editor.Commit();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  jmp block3\n"
      "block3:\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirInstructionsTestX64, LiteralInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewLiteralInstruction(
      factory()->NewRegister(),
      factory()->NewIntValue(Value::Size::Size64, 42)));
  editor.Append(factory()->NewLiteralInstruction(
      factory()->NewRegister(), factory()->NewStringValue(L"foo")));
  editor.Commit();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  mov %r1l = 42l\n"
      "  mov %r2l = \"foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirInstructionsTestX64, LoadInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const destination = factory()->NewRegister();
  editor.Append(factory()->NewLoadInstruction(
      destination, Value::Parameter(destination.type, destination.size, 4)));
  editor.Commit();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  load %r1l = %param[4]\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

}  // namespace lir
}  // namespace elang
