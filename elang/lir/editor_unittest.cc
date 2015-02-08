// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
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
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  // false block
  editor.Edit(false_block);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  // merge block
  editor.Edit(merge_block);
  auto const merge_phi = editor.NewPhi(values[2]);
  editor.SetPhiInput(merge_phi, true_block, values[1]);
  editor.SetPhiInput(merge_phi, false_block,
                     factory()->NewIntValue(ValueSize::Size32, 42));
  editor.Append(
      factory()->NewCopyInstruction(Target::GetReturn(values[0]), values[2]));
  editor.SetReturn();
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

TEST_F(LirEditorTest, InsertAfter) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);

  editor.Edit(function->entry_block());
  auto const ref_instr = factory()->NewLiteralInstruction(
      factory()->NewRegister(), factory()->NewIntValue(ValueSize::Size64, 42));
  editor.Append(ref_instr);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(function->entry_block());
  auto const new_instr = factory()->NewCopyInstruction(factory()->NewRegister(),
                                                       ref_instr->output(0));
  editor.InsertAfter(new_instr, ref_instr);
  EXPECT_EQ("", Commit(&editor));

  EXPECT_EQ(ref_instr, new_instr->previous());
  EXPECT_EQ(new_instr, ref_instr->next());
}

TEST_F(LirEditorTest, InsertCopyBefore) {
  auto const function = CreateFunctionEmptySample();
  auto const entry_block = function->entry_block();
  auto const last_instruction = entry_block->last_instruction();
  Editor editor(factory(), function);
  editor.Edit(entry_block);
  auto const register1 = factory()->NewRegister();
  auto const register2 = factory()->NewRegister();
  editor.InsertCopyBefore(register1, register2, last_instruction);
  editor.InsertCopyBefore(register2, register1, last_instruction);
  EXPECT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  mov %r1l = %r2l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
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

// The sample block for RemoveCriticalEdges test case:
//   entry:
//    br start
//   start:
//    br sample
//   sample:
//    br %flag1, merge, start
//   merge:
//    phi %1 = start 39, sample 42
//    ret
// An edge sample => merge is a critical edge.
//
TEST_F(LirEditorTest, RemoveCriticalEdges) {
  auto const function = CreateFunctionEmptySample();
  auto const entry_block = function->entry_block();
  auto const exit_block = function->exit_block();

  Editor editor(factory(), function);

  Value type(Value::Type::Integer, ValueSize::Size32);
  auto const start_block = editor.NewBasicBlock(exit_block);
  auto const sample_block = editor.NewBasicBlock(exit_block);
  auto const merge_block = editor.NewBasicBlock(exit_block);

  editor.Edit(entry_block);
  editor.SetJump(start_block);
  ASSERT_EQ("", Commit(&editor));

  editor.Edit(start_block);
  editor.SetJump(merge_block);
  ASSERT_EQ("", Commit(&editor));

  editor.Edit(sample_block);
  editor.SetBranch(factory()->NewCondition(), merge_block, start_block);
  ASSERT_EQ("", Commit(&editor));

  editor.Edit(merge_block);
  auto const phi_instr = editor.NewPhi(NewRegister(type));
  editor.SetPhiInput(phi_instr, sample_block, Value::SmallInt32(42));
  editor.SetPhiInput(phi_instr, start_block, Value::SmallInt32(39));
  editor.SetReturn();
  ASSERT_EQ("", Commit(&editor));

  ASSERT_EQ("", Validate(&editor));
  editor.RemoveCriticalEdges();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3}\n"
      "  entry\n"
      "  jmp block3\n"
      "block3:\n"
      "  // In: {block1, block4}\n"
      "  // Out: {block5}\n"
      "  jmp block5\n"
      "block4:\n"
      "  // In: {}\n"
      "  // Out: {block3, block6}\n"
      "  br %b2, block6, block3\n"
      "block6:\n"  // a new block introduced by |RemoveCriticalEdges()|.
      "  // In: {block4}\n"
      "  // Out: {block5}\n"
      "  jmp block5\n"
      "block5:\n"
      "  // In: {block3, block6}\n"
      "  // Out: {block2}\n"
      "  phi %r1 = block6 42, block3 39\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block5}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(LirEditorTest, Replace) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);

  auto const entry_block = function->entry_block();
  editor.Edit(entry_block);
  auto const ref_instr = factory()->NewLiteralInstruction(
      factory()->NewRegister(), factory()->NewIntValue(ValueSize::Size64, 42));
  editor.Append(ref_instr);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(entry_block);
  auto const new_instr = factory()->NewCopyInstruction(factory()->NewRegister(),
                                                       ref_instr->output(0));
  editor.Replace(new_instr, ref_instr);
  EXPECT_EQ("", Commit(&editor));

  EXPECT_EQ(entry_block->last_instruction(), new_instr->next());
  EXPECT_EQ(entry_block->first_instruction(), new_instr->previous());

  EXPECT_EQ(0, ref_instr->id());
  EXPECT_EQ(nullptr, ref_instr->next());
  EXPECT_EQ(nullptr, ref_instr->previous());
}

TEST_F(LirEditorTest, SetJump) {
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

}  // namespace lir
}  // namespace elang
