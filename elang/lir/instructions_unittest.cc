// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirInstructionTest offers HIR factories.
//
class LirInstructionTest : public ::testing::Test {
 protected:
  LirInstructionTest();

  Factory* factory() { return factory_.get(); }

 private:
  const std::unique_ptr<Factory> factory_;
};

LirInstructionTest::LirInstructionTest() : factory_(new Factory()) {
}

// CallInstruction
TEST_F(LirInstructionTest, CallInstruction) {
  auto const callee = factory()->NewStringValue(L"Foo");
  auto const instr = factory()->NewCallInstruction(callee);
  EXPECT_TRUE(instr->is<CallInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

// CopyInstruction
TEST_F(LirInstructionTest, CopyInstruction) {
  auto const instr = factory()->NewCopyInstruction(factory()->NewRegister(),
                                                   factory()->NewRegister());
  EXPECT_TRUE(instr->is<CopyInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
}

// EntryInstruction
TEST_F(LirInstructionTest, EntryInstruction) {
  auto const instr = factory()->NewEntryInstruction();
  EXPECT_TRUE(instr->is<EntryInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(0, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

// ExitInstruction
TEST_F(LirInstructionTest, ExitInstruction) {
  auto const instr = factory()->NewExitInstruction();
  EXPECT_TRUE(instr->is<ExitInstruction>());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(0, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

// JumpInstruction
TEST_F(LirInstructionTest, JumpInstruction) {
  auto const function = factory()->NewFunction();
  auto const instr = factory()->NewJumpInstruction(function->exit_block());
  EXPECT_TRUE(instr->is<JumpInstruction>());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

// LoadInstruction
TEST_F(LirInstructionTest, LoadInstruction) {
  auto const destination = factory()->NewRegister();
  auto const instr = factory()->NewLoadInstruction(
      destination, Value::Parameter(destination.type, destination.size, 0));
  EXPECT_TRUE(instr->is<LoadInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
}

// RetInstruction
TEST_F(LirInstructionTest, RetInstruction) {
  auto const instr = factory()->NewRetInstruction();
  EXPECT_TRUE(instr->is<RetInstruction>());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(0, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

}  // namespace lir
}  // namespace elang
