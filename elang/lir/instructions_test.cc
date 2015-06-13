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
  std::ostringstream ostream;
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
  auto const instr = factory()->NewCallInstruction({}, callee);
  EXPECT_TRUE(instr->is<CallInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

TEST_F(LirInstructionTest, CallInstruction2) {
  auto const callee = factory()->NewStringValue(L"Foo");
  auto const output = NewRegister(Value::Int32Type());
  auto const instr = factory()->NewCallInstruction({output}, callee);
  EXPECT_TRUE(instr->is<CallInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
}

// CmpInstruction
TEST_F(LirInstructionTest, CmpInstruction) {
  auto const left = NewRegister(Value::Int32Type());
  auto const right = NewRegister(Value::Int32Type());
  auto const instr =
      NewCmpInstruction(NewConditional(), IntCondition::NotEqual, left, right);
  EXPECT_TRUE(instr->is<CmpInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
  EXPECT_EQ("--:0:cmp_ne %b2 = %r1, %r2", ToString(*instr));
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
      Target::ParameterAt(Value::Int32Type(), 0),
      Target::ParameterAt(Value::Int64Type(), 1),
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

// FloatCmpInstruction
TEST_F(LirInstructionTest, FloatCmpInstruction) {
  auto const left = NewRegister(Value::Float32Type());
  auto const right = NewRegister(Value::Float32Type());
  auto const instr = NewFloatCmpInstruction(
      NewConditional(), FloatCondition::OrderedNotEqual, left, right);
  EXPECT_TRUE(instr->is<FloatCmpInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
  EXPECT_EQ("--:0:FloatCmp_ne %b2 = %f1, %f2", ToString(*instr));
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
  auto const array = NewRegister(Target::IntPtrType());
  auto const pointer = NewRegister(Target::IntPtrType());
  auto const offset = Value::SmallInt32(42);
  auto const output = NewRegister(Value::Int32Type());
  auto const instr =
      factory()->NewLoadInstruction(output, array, pointer, offset);
  EXPECT_TRUE(instr->is<LoadInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(3, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
  EXPECT_EQ("--:0:load %r3 = %r1l, %r2l, 42", ToString(*instr));
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
  auto const anchor = NewIntPtrRegister();
  auto const pointer = anchor;
  auto const offset = Value::SmallInt32(4);
  auto const value = Value::SmallInt8(42);
  auto const instr =
      factory()->New<StoreInstruction>(anchor, pointer, offset, value);
  EXPECT_TRUE(instr->is<StoreInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(4, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
}

// UseInstruction
TEST_F(LirInstructionTest, UseInstruction) {
  auto const input = NewRegister(Value::Int32Type());
  auto const instr = factory()->NewUseInstruction(input);
  EXPECT_TRUE(instr->is<UseInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(1, instr->inputs().size());
  EXPECT_EQ(0, instr->outputs().size());
  EXPECT_EQ("--:0:use %r1", ToString(*instr));
}

// UIntDivInstruction
TEST_F(LirInstructionTest, UIntDivInstruction) {
  auto const input = NewIntPtrRegister();
  auto const output = NewIntPtrRegister();
  auto const instr =
      factory()->NewUIntDivInstruction(output, input, Value::SmallInt32(3));
  EXPECT_TRUE(instr->is<UIntDivInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
  EXPECT_EQ("--:0:udiv %r2l = %r1l, 3", ToString(*instr));
}

// UIntModInstruction
TEST_F(LirInstructionTest, UIntModInstruction) {
  auto const input = NewIntPtrRegister();
  auto const output = NewIntPtrRegister();
  auto const instr =
      factory()->NewUIntModInstruction(output, input, Value::SmallInt32(3));
  EXPECT_TRUE(instr->is<UIntModInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
  EXPECT_EQ("--:0:umod %r2l = %r1l, 3", ToString(*instr));
}

// UIntShrInstruction
TEST_F(LirInstructionTest, UIntShrInstruction) {
  auto const input = NewIntPtrRegister();
  auto const output = NewIntPtrRegister();
  auto const instr =
      factory()->NewUIntShrInstruction(output, input, Value::SmallInt32(3));
  EXPECT_TRUE(instr->is<UIntShrInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
  EXPECT_EQ(2, instr->inputs().size());
  EXPECT_EQ(1, instr->outputs().size());
  EXPECT_EQ("--:0:ushr %r2l = %r1l, 3", ToString(*instr));
}

}  // namespace lir
}  // namespace elang
