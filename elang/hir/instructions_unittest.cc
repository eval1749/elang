// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/testing/hir_test.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// HirInstructionTest offers HIR factories.
//
class HirInstructionTest : public testing::HirTest {
 protected:
  HirInstructionTest() = default;
  ~HirInstructionTest() override = default;

  Instruction* NewConsumer(Type* input_type);
  Instruction* NewSource(Type* output_type);

 private:
  DISALLOW_COPY_AND_ASSIGN(HirInstructionTest);
};

Instruction* HirInstructionTest::NewConsumer(Type* input_type) {
  auto const name = factory()->NewAtomicString(L"Consumer");
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(void_type(), input_type), name);
  return factory()->NewCallInstruction(callee, input_type->default_value());
}

Instruction* HirInstructionTest::NewSource(Type* output_type) {
  auto const name = factory()->NewAtomicString(L"Source");
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(output_type, void_type()), name);
  return factory()->NewCallInstruction(callee, void_value());
}

//////////////////////////////////////////////////////////////////////
//
// BranchInstruction
//
// Conditional
TEST_F(HirInstructionTest, BranchInstruction) {
  auto const true_block = editor()->EditNewBasicBlock();
  editor()->SetReturn(void_value());
  editor()->Commit();

  auto const false_block = editor()->EditNewBasicBlock();
  editor()->SetReturn(void_value());
  editor()->Commit();

  editor()->Edit(entry_block());
  auto const call_instr = NewSource(bool_type());
  editor()->Append(call_instr);
  editor()->SetBranch(call_instr, true_block, false_block);
  editor()->Commit();

  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(types()->void_type(), instr->output_type());
  EXPECT_EQ(3, instr->CountInputs());
  EXPECT_EQ(call_instr, instr->input(0));
  EXPECT_EQ(true_block, instr->input(1));
  EXPECT_EQ(false_block, instr->input(2));
}

TEST_F(HirInstructionTest, BranchUncoditional) {
  auto const target_block = editor()->EditNewBasicBlock();
  editor()->SetReturn(void_value());
  editor()->Commit();

  editor()->Edit(entry_block());
  editor()->SetBranch(target_block);
  editor()->Commit();

  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(types()->void_type(), instr->output_type());
  EXPECT_EQ(1, instr->CountInputs());
  EXPECT_EQ(target_block, instr->input(0));
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
TEST_F(HirInstructionTest, CallInstruction) {
  auto const string_type = types()->GetStringType();
  auto const callee_name = factory()->NewAtomicString(L"Console.WriteLine");
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(void_type(), string_type), callee_name);
  auto const args = factory()->NewStringLiteral(L"Hello world!");
  auto const instr = factory()->NewCallInstruction(callee, args);
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ(callee, instr->input(0));
  EXPECT_EQ(args, instr->input(1));

  auto callee_found = false;
  for (auto const user : callee->users()) {
    if (user->instruction() == instr) {
      callee_found = true;
      break;
    }
  }
  EXPECT_TRUE(callee_found) << "call instruction must be a user of callee.";
}

//////////////////////////////////////////////////////////////////////
//
// LoadInstruction
//
TEST_F(HirInstructionTest, LoadInstruction) {
  auto const bool_pointer_type = types()->NewPointerType(bool_type());
  auto const source = NewSource(bool_pointer_type);
  auto const instr = factory()->NewLoadInstruction(source);
  EXPECT_TRUE(instr->CanBeRemoved());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(bool_type(), instr->output_type());
  EXPECT_EQ(1, instr->CountInputs());
  EXPECT_EQ(source, instr->input(0));
}

//////////////////////////////////////////////////////////////////////
//
// PhiInstruction
//
TEST_F(HirInstructionTest, PhiInstruction) {
  // Create diamond graph
  auto const merge_block =
      editor()->SplitBefore(entry_block()->last_instruction());

  auto const true_block = editor()->EditNewBasicBlock(merge_block);
  editor()->SetBranch(merge_block);
  ASSERT_TRUE(editor()->Commit()) << GetErrors();

  auto const false_block = editor()->EditNewBasicBlock(merge_block);
  editor()->SetBranch(merge_block);
  ASSERT_TRUE(editor()->Commit()) << GetErrors();

  editor()->Continue(entry_block());
  auto const call_instr = NewSource(bool_type());
  editor()->Append(call_instr);
  editor()->SetBranch(call_instr, true_block, false_block);
  ASSERT_TRUE(editor()->Commit()) << GetErrors();

  editor()->Edit(merge_block);
  auto const phi = editor()->NewPhi(bool_type());
  editor()->SetPhiInput(phi, true_block, NewBool(true));
  editor()->SetPhiInput(phi, false_block, NewBool(false));
  auto const consumer = NewConsumer(bool_type());
  editor()->Append(consumer);
  editor()->SetInput(consumer, 1, phi);
  ASSERT_TRUE(editor()->Commit()) << GetErrors();

  EXPECT_EQ(
      "function1 void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block5\n"
      "  entry\n"
      "  bool %b6 = call `Source`, void\n"
      "  br %b6, block4, block5\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  br block3\n"
      "block5:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block5\n"
      "  // Out: block2\n"
      "  bool %b8 = phi block4 true, block5 false\n"
      "  call `Consumer`, %b8\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Format());
}

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
TEST_F(HirInstructionTest, ReturnInstruction) {
  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(types()->void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ(void_value(), instr->input(0));
  EXPECT_EQ(exit_block(), instr->input(1));
}

//////////////////////////////////////////////////////////////////////
//
// StoreInstruction
//
TEST_F(HirInstructionTest, StoreInstruction) {
  auto const bool_pointer_type = types()->NewPointerType(bool_type());
  auto const source = NewSource(bool_pointer_type);
  auto const value = types()->GetBoolType()->default_value();
  auto const instr = factory()->NewStoreInstruction(source, value);
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ(source, instr->input(0));
  EXPECT_EQ(value, instr->input(1));
}

}  // namespace hir
}  // namespace elang
