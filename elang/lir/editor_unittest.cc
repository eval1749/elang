// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirEditorTest
//
class LirEditorTest : public testing::LirTest {
 protected:
  LirEditorTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirEditorTest);
};

// Test cases...

// int Foo(int x, int y) {
//   return x == 0 ? y : 42;
// }
TEST_F(LirEditorTest, AnalyzeLiveness) {
  auto const function = CreateFunctionEmptySample();
  auto const exit_block = function->exit_block();
  Editor editor(factory(), function);
  auto const true_block = editor.NewBasicBlock(exit_block);
  auto const false_block = editor.NewBasicBlock(exit_block);
  auto const merge_block = editor.NewBasicBlock(exit_block);

  std::vector<Value> values{
      factory()->NewRegister(ValueSize::Size32),
      factory()->NewRegister(ValueSize::Size32),
      factory()->NewRegister(ValueSize::Size32),
  };

  // merge block
  editor.Edit(merge_block);
  auto const merge_phi = editor.NewPhi(values[2]);
  editor.Append(
      factory()->NewCopyInstruction(Target::GetReturn(values[0]), values[2]));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));

  // entry block
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewPCopyInstruction(
      {values[0], values[1]}, {Target::GetParameterAt(values[0], 0),
                               Target::GetParameterAt(values[1], 1)}));
  auto const cond1 = factory()->NewCondition();
  editor.Append(factory()->NewEqInstruction(
      cond1, values[0], factory()->NewIntValue(ValueSize::Size32, 0)));
  editor.SetBranch(cond1, true_block, false_block);
  EXPECT_EQ("", Commit(&editor));

  // true block
  editor.Edit(true_block);
  editor.SetPhiInput(merge_phi, true_block, values[1]);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  // false block
  editor.Edit(false_block);
  editor.SetPhiInput(merge_phi, false_block,
                     factory()->NewIntValue(ValueSize::Size32, 42));
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  auto const& collection = editor.AnalyzeLiveness();

  auto& true_liveness = collection.LivenessOf(true_block);
  EXPECT_FALSE(true_liveness.in().Contains(collection.NumberOf(values[0])));
  EXPECT_TRUE(true_liveness.in().Contains(collection.NumberOf(values[1])));

  auto& false_liveness = collection.LivenessOf(false_block);
  EXPECT_FALSE(false_liveness.in().Contains(collection.NumberOf(values[0])));
  EXPECT_TRUE(false_liveness.in().Contains(collection.NumberOf(values[1])));

  auto& merge_liveness = collection.LivenessOf(merge_block);
  EXPECT_FALSE(merge_liveness.in().Contains(collection.NumberOf(values[0])));
  EXPECT_TRUE(merge_liveness.in().Contains(collection.NumberOf(values[1])));
}

TEST_F(LirEditorTest, FunctionEmpty) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(LirEditorTest, FunctionSample1) {
  auto const function = CreateFunctionSample1();
  Editor editor(factory(), function);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  call \"Foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(LirEditorTest, JumpInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  auto const block = editor.NewBasicBlock(function->exit_block());
  editor.Edit(block);
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));
  editor.Edit(function->entry_block());
  editor.SetJump(block);
  EXPECT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3}\n"
      "  entry\n"
      "  jmp block3\n"
      "block3:\n"
      "  // In: {block1}\n"
      "  // Out: {block2}\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block3}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(LirEditorTest, LiteralInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewLiteralInstruction(
      factory()->NewRegister(), factory()->NewIntValue(ValueSize::Size64, 42)));
  editor.Append(factory()->NewLiteralInstruction(
      factory()->NewRegister(), factory()->NewStringValue(L"foo")));
  EXPECT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  mov %r1l = 42l\n"
      "  mov %r2l = \"foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

}  // namespace lir
}  // namespace elang
