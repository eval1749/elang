// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/base/analysis/dominator_tree.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

namespace {
//////////////////////////////////////////////////////////////////////
//
// DominatorTreeWalker visits child nodes in dominator tree.
//
struct DominatorTreeWalker {
  std::unordered_set<BasicBlock*> visited;
  const DominatorTree<Function>* dominator_tree;

  explicit DominatorTreeWalker(const DominatorTree<Function>* dominator_tree)
      : dominator_tree(dominator_tree) {}

  bool IsVisited(BasicBlock* block) const { return visited.count(block); }

  void Visit(BasicBlock* block) {
    EXPECT_FALSE(IsVisited(block)) << "Dominator tree contains " << *block
                                   << " more than once.";
    visited.insert(block);
    for (auto const child_node :
         dominator_tree->TreeNodeOf(block)->children()) {
      Visit(child_node->value());
    }
  }
};
}  // namespace

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
  auto const function = CreateFunctionSample2();
  auto const entry_block = function->entry_block();
  auto const values = CollectRegisters(function);
  auto const true_block = entry_block->last_instruction()->block_operand(0);
  auto const false_block = entry_block->last_instruction()->block_operand(1);
  auto const merge_block = true_block->last_instruction()->block_operand(0);

  Editor editor(factory(), function);
  auto const& collection = editor.AnalyzeLiveness();

  auto& true_liveness = collection.LivenessOf(true_block);
  EXPECT_FALSE(true_liveness.in().Contains(collection.NumberOf(values[0])));
  EXPECT_TRUE(true_liveness.in().Contains(collection.NumberOf(values[1])));

  auto& false_liveness = collection.LivenessOf(false_block);
  EXPECT_FALSE(false_liveness.in().Contains(collection.NumberOf(values[0])));
  EXPECT_FALSE(false_liveness.in().Contains(collection.NumberOf(values[1])));

  auto& merge_liveness = collection.LivenessOf(merge_block);
  EXPECT_FALSE(merge_liveness.in().Contains(collection.NumberOf(values[0])));
  EXPECT_FALSE(merge_liveness.in().Contains(collection.NumberOf(values[1])));
}

TEST_F(LirEditorTest, AssignIndex) {
  auto const function = CreateFunctionEmptySample();
  auto const entry_block = function->entry_block();
  auto const last_instruction = entry_block->last_instruction();
  Editor editor(factory(), function);
  editor.Edit(entry_block);
  auto const register1 = factory()->NewRegister();
  auto const register2 = factory()->NewRegister();
  editor.InsertCopyBefore(register1, register2, last_instruction);
  EXPECT_EQ("", Commit(&editor));
  ASSERT_EQ(
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

  auto counters = editor.AssignIndex();
  EXPECT_EQ(2, counters.block_counter);
  EXPECT_EQ(4, counters.instruction_counter);
  EXPECT_EQ(1, counters.output_counter);
}

TEST_F(LirEditorTest, BuildDominatorTree) {
  auto const function = CreateFunctionSample2();
  Editor editor(factory(), function);
  auto const& dominator_tree = editor.BuildDominatorTree();
  DominatorTreeWalker walker(&dominator_tree);
  walker.Visit(function->entry_block());
  for (auto const block : function->basic_blocks())
    EXPECT_TRUE(walker.IsVisited(block)) << *block
                                         << " is not in dominator tree.";
}

TEST_F(LirEditorTest, BuildPostDominatorTree) {
  auto const function = CreateFunctionSample2();
  Editor editor(factory(), function);
  auto const& dominator_tree = editor.BuildPostDominatorTree();
  DominatorTreeWalker walker(&dominator_tree);
  walker.Visit(function->exit_block());
  for (auto const block : function->basic_blocks())
    EXPECT_TRUE(walker.IsVisited(block)) << *block
                                         << " is not in dominator tree.";
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
      "  lit %r1l = 42l\n"
      "  lit %r2l = \"foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

// Before removing critical edges:
//    function1:
//    block1:
//      // In: {}
//      // Out: {block3}
//      entry
//      jmp block3
//    block3:
//      // In: {block1, block4}
//      // Out: {block4, block5}
//      br %b2, block5, block4
//    block4:
//      // In: {block3}
//      // Out: {block3, block6}
//      br %b3, block6, block3
//    block5:
//      // In: {block3}
//      // Out: {block6}
//      jmp block6
//    block6:
//      // In: {block4, block5}
//      // Out: {block2}
//      phi %r1 = block4 42, block5 39
//      mov EAX = %r1
//      ret block2
//    block2:
//      // In: {block6}
//      // Out: {}
//      exit
// Edge block4=>block6 is a critical edge.
TEST_F(LirEditorTest, RemoveCriticalEdges) {
  auto const function = CreateFunctionWithCriticalEdge();
  Editor editor(factory(), function);
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
      "  // Out: {block4, block5}\n"
      "  br %b2, block5, block4\n"
      "block4:\n"
      "  // In: {block3}\n"
      "  // Out: {block3, block7}\n"
      "  br %b3, block7, block3\n"
      "block5:\n"
      "  // In: {block3}\n"
      "  // Out: {block6}\n"
      "  jmp block6\n"
      "block7:\n"  // a new block introduced by |RemoveCriticalEdges()|.
      "  // In: {block4}\n"
      "  // Out: {block6}\n"
      "  jmp block6\n"
      "block6:\n"
      "  // In: {block5, block7}\n"
      "  // Out: {block2}\n"
      "  phi %r1 = block7 42, block5 39\n"
      "  mov EAX = %r1\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block6}\n"
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

TEST_F(LirEditorTest, ReplacePhiInputs) {
  auto const function = CreateFunctionWithCriticalEdge();
  Editor editor(factory(), function);
  std::vector<BasicBlock*> blocks(function->basic_blocks().begin(),
                                  function->basic_blocks().end());
  auto const merge_block = blocks[4];
  ASSERT_FALSE(merge_block->phi_instructions().empty());
  auto const sample_block = blocks[2];
  auto const new_block = editor.NewBasicBlock(merge_block);

  editor.Edit(new_block);
  editor.SetJump(merge_block);
  editor.Commit();

  editor.Edit(sample_block);
  auto const branch = sample_block->last_instruction();
  editor.SetBlockOperand(branch, 0, new_block);
  editor.Commit();

  editor.Edit(merge_block);
  editor.ReplacePhiInputs(new_block, sample_block);
  editor.Commit();

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3}\n"
      "  entry\n"
      "  jmp block3\n"
      "block3:\n"
      "  // In: {block1, block4}\n"
      "  // Out: {block4, block5}\n"
      "  br %b2, block5, block4\n"
      "block4:\n"
      "  // In: {block3}\n"
      "  // Out: {block3, block7}\n"
      "  br %b3, block7, block3\n"
      "block5:\n"
      "  // In: {block3}\n"
      "  // Out: {block6}\n"
      "  jmp block6\n"
      "block7:\n"
      "  // In: {block4}\n"
      "  // Out: {block6}\n"
      "  jmp block6\n"
      "block6:\n"
      "  // In: {block5, block7}\n"
      "  // Out: {block2}\n"
      "  phi %r1 = block7 42, block5 39\n"
      "  mov EAX = %r1\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block6}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

}  // namespace lir
}  // namespace elang
