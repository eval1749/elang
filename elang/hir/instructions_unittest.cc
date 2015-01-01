// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "gtest/gtest.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// InstructionTest offers HIR factories.
//
class InstructionTest : public ::testing::Test {
 protected:
  InstructionTest();

  Factory* factory() { return factory_.get(); }
  TypeFactory* types() { return factory_->types(); }
  Zone* zone() { return factory_->zone(); }

 private:
  std::unique_ptr<Factory> factory_;
};

InstructionTest::InstructionTest() : factory_(new Factory()) {
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
TEST_F(InstructionTest, CallInstruction) {
  auto const void_type = types()->GetVoidType();
  auto const string_type = types()->GetStringType();
  auto const callee = string_type->NewLiteral(L"Console.WriteLine");
  auto const args = string_type->NewLiteral(L"Hello world!");
  auto const instr = CallInstruction::New(factory(), void_type, callee, args);
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_FALSE(instr->IsExit());
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
TEST_F(InstructionTest, ReturnInstruction) {
  auto const void_value = factory()->GetVoid();
  auto const instr = ReturnInstruction::New(factory(), void_value);
  EXPECT_FALSE(instr->CanBeRemoved());
  EXPECT_TRUE(instr->IsExit());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(types()->GetVoidType(), instr->output_type());
  EXPECT_EQ(1, instr->CountOperands());
  EXPECT_EQ(void_value, instr->OperandAt(0));
}

}  // namespace hir
}  // namespace elang
