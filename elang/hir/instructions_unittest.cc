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

namespace {
Function* NewSampleFunction(Factory* factory) {
  auto const types = factory->types();
  auto const function_type =
      types->NewFunctionType(types->void_type(), types->void_type());
  return factory->NewFunction(function_type);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// HirInstructionTest offers HIR factories.
//
class HirInstructionTest : public testing::HirTest {
 protected:
  HirInstructionTest();
  ~HirInstructionTest() override = default;

  BoolType* bool_type() { return factory()->types()->GetBoolType(); }
  BasicBlock* entry_block() { return function_->entry_block(); }
  BasicBlock* exit_block() { return function_->exit_block(); }
  Function* function() const { return function_; }
  VoidType* void_type() { return factory()->types()->void_type(); }
  VoidValue* void_value() { return factory()->void_value(); }

 private:
  Function* function_;

  DISALLOW_COPY_AND_ASSIGN(HirInstructionTest);
};

HirInstructionTest::HirInstructionTest()
    : function_(NewSampleFunction(factory())) {
}

//////////////////////////////////////////////////////////////////////
//
// BranchInstruction
//
TEST_F(HirInstructionTest, BranchInstruction) {
  Editor editor(factory(), function());
  auto const true_block = editor.NewBasicBlock();
  editor.SetReturn(void_value());

  auto const false_block = editor.NewBasicBlock();
  editor.SetReturn(void_value());

  editor.Edit(entry_block());
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(bool_type(), void_type()), L"Foo");
  auto const call_instr = static_cast<Instruction*>(
      factory()->NewCallInstruction(bool_type(), callee, void_value()));
  editor.SetBranch(call_instr, true_block, false_block);
  editor.Commit();

  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(types()->void_type(), instr->output_type());
  EXPECT_EQ(3, instr->CountOperands());
  EXPECT_EQ(call_instr, instr->OperandAt(0));
  EXPECT_EQ(true_block, instr->OperandAt(1));
  EXPECT_EQ(false_block, instr->OperandAt(2));
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
TEST_F(HirInstructionTest, CallInstruction) {
  auto const string_type = types()->GetStringType();
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(void_type(), string_type), L"Console.WriteLine");
  auto const args = factory()->NewStringLiteral(L"Hello world!");
  auto const instr = static_cast<Instruction*>(
      factory()->NewCallInstruction(void_type(), callee, args));
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountOperands());
  EXPECT_EQ(callee, instr->OperandAt(0));
  EXPECT_EQ(args, instr->OperandAt(1));

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
// JumpInstruction
//
TEST_F(HirInstructionTest, JumpInstruction) {
  Editor editor(factory(), function());
  auto const target_block = editor.NewBasicBlock();
  editor.SetReturn(void_value());

  editor.Edit(entry_block());
  editor.SetJump(target_block);
  editor.Commit();

  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(types()->void_type(), instr->output_type());
  EXPECT_EQ(1, instr->CountOperands());
  EXPECT_EQ(target_block, instr->OperandAt(0));
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
  EXPECT_EQ(2, instr->CountOperands());
  EXPECT_EQ(void_value(), instr->OperandAt(0));
  EXPECT_EQ(exit_block(), instr->OperandAt(1));
}

}  // namespace hir
}  // namespace elang
