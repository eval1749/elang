// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>
#include <vector>

#include "elang/lir/testing/lir_test.h"

#include "elang/base/zone.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirInstructionTest
//
class LirInstructionTest : public testing::LirTest {
 protected:
  LirInstructionTest() = default;
  ~LirInstructionTest() = default;

  Value NewIntPtrRegister();
  static std::string ToString(const Instruction& instr);
};

Value LirInstructionTest::NewIntPtrRegister() {
  return NewRegister(Target::IntPtrType());
}

std::string LirInstructionTest::ToString(const Instruction& instr) {
  std::stringstream ostream;
  ostream << instr;
  return ostream.str();
}

// Test cases...

// AssignInstruction
TEST_F(LirInstructionTest, AssignInstruction) {
  auto const instr =
      factory()->NewAssignInstruction(NewIntPtrRegister(), NewIntPtrRegister());
  EXPECT_TRUE(instr->is<AssignInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
}

// BranchInstruction
TEST_F(LirInstructionTest, BranchInstruction) {
  auto const function = factory()->NewFunction({});
  auto const entry_block = function->entry_block();
  auto const exit_block = function->exit_block();
  auto const instr =
      factory()->NewBranchInstruction(Value::True(), entry_block, exit_block);
  EXPECT_TRUE(instr->is<BranchInstruction>());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(Value::True(), instr->input(0));
  EXPECT_EQ(0, instr->outputs().size());
  EXPECT_EQ(entry_block, instr->as<BranchInstruction>()->true_block());
  EXPECT_EQ(exit_block, instr->as<BranchInstruction>()->false_block());
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

// CmpInstruction
TEST_F(LirInstructionTest, CmpInstruction) {
  auto const instr = NewCmpInstruction(
      NewConditional(), IntegerCondition::NotEqual,
      NewRegister(Value::Int32Type()), NewRegister(Value::Int32Type()));
  EXPECT_TRUE(instr->is<CmpInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
  EXPECT_EQ("--:0:cmp_ne %b2 = %r2, %r1", ToString(*instr));
}

// CopyInstruction
TEST_F(LirInstructionTest, CopyInstruction) {
  auto const instr =
      factory()->NewCopyInstruction(NewIntPtrRegister(), NewIntPtrRegister());
  EXPECT_TRUE(instr->is<CopyInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
}

// EntryInstruction
TEST_F(LirInstructionTest, EntryInstruction) {
  auto const instr = factory()->NewEntryInstruction({});
  EXPECT_TRUE(instr->is<EntryInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(0, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

TEST_F(LirInstructionTest, EntryInstruction2) {
  std::vector<Value> parameters{
      Target::GetParameterAt(Value::Int32Type(), 0),
      Target::GetParameterAt(Value::Int64Type(), 1),
  };
  auto const instr = factory()->NewEntryInstruction(parameters);
  EXPECT_TRUE(instr->is<EntryInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(0, instr->inputs().size());
  EXPECT_EQ(2, instr->outputs().size());
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
  auto const function = factory()->NewFunction({});
  auto const exit_block = function->exit_block();
  auto const instr = factory()->NewJumpInstruction(exit_block);
  EXPECT_TRUE(instr->is<JumpInstruction>());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(0, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
  EXPECT_EQ(exit_block, instr->as<JumpInstruction>()->target_block());
}

// LoadInstruction
TEST_F(LirInstructionTest, LoadInstruction) {
  auto const destination = NewIntPtrRegister();
  auto const instr = factory()->NewLoadInstruction(
      destination, Value::Parameter(destination, 0));
  EXPECT_TRUE(instr->is<LoadInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
}

// PCopyInstruction
TEST_F(LirInstructionTest, PCopyInstruction) {
  std::vector<Value> outputs{
      NewIntPtrRegister(), NewIntPtrRegister(),
  };
  std::vector<Value> inputs{
      factory()->NewIntValue(outputs.front(), 42), NewIntPtrRegister(),
  };
  auto const instr = factory()->NewPCopyInstruction(outputs, inputs);
  EXPECT_TRUE(instr->is<PCopyInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(2, instr->outputs().size());
}

// RetInstruction
TEST_F(LirInstructionTest, RetInstruction) {
  auto const function = factory()->NewFunction({});
  auto const instr = function->entry_block()->last_instruction();
  EXPECT_TRUE(instr->is<RetInstruction>());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_NE(0, instr->id());
  EXPECT_EQ(0, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

// StoreInstruction
TEST_F(LirInstructionTest, StoreInstruction) {
  auto const source = NewIntPtrRegister();
  auto const instr =
      factory()->NewStoreInstruction(Value::Argument(source, 0), source);
  EXPECT_TRUE(instr->is<StoreInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

// UShrInstruction
TEST_F(LirInstructionTest, UShrInstruction) {
  auto const input = NewIntPtrRegister();
  auto const output = NewIntPtrRegister();
  auto const instr =
      factory()->NewUShrInstruction(output, input, Value::SmallInt32(3));
  EXPECT_TRUE(instr->is<UShrInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
}

}  // namespace lir
}  // namespace elang
