// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "gtest/gtest.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// HirInstructionTest offers HIR factories.
//
class HirInstructionTest : public ::testing::Test {
 protected:
  HirInstructionTest();

  Factory* factory() { return factory_.get(); }
  TypeFactory* types() { return factory_->types(); }
  Zone* zone() { return factory_->zone(); }

 private:
  std::unique_ptr<Factory> factory_;
};

HirInstructionTest::HirInstructionTest() : factory_(new Factory()) {
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
TEST_F(HirInstructionTest, CallInstruction) {
  auto const void_type = types()->GetVoidType();
  auto const string_type = types()->GetStringType();
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(void_type, string_type), L"Console.WriteLine");
  auto const args = factory()->NewStringLiteral(L"Hello world!");
  auto const instr = static_cast<Instruction*>(
      CallInstruction::New(factory(), void_type, callee, args));
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(void_type, instr->output_type());
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
// ReturnInstruction
//
TEST_F(HirInstructionTest, ReturnInstruction) {
  auto const void_type = factory()->GetVoidType();
  auto const void_value = factory()->GetVoidValue();

  auto const function_type = types()->NewFunctionType(void_type, void_type);
  auto const function = factory()->NewFunction(function_type);

  auto const instr = function->entry_block()->last_instruction();
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(types()->GetVoidType(), instr->output_type());
  EXPECT_EQ(2, instr->CountOperands());
  EXPECT_EQ(void_value, instr->OperandAt(0));
  EXPECT_EQ(function->exit_block(), instr->OperandAt(1));
}

}  // namespace hir
}  // namespace elang
