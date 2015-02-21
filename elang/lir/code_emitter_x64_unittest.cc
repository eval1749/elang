// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/code_emitter.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/testing/test_machine_code_builder.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {
namespace {

//////////////////////////////////////////////////////////////////////
//
// CodeEmitterX64Test
//
class CodeEmitterX64Test : public testing::LirTest {
 protected:
  CodeEmitterX64Test() = default;
  ~CodeEmitterX64Test() override = default;

  std::string Emit(Function* function);

 private:
  DISALLOW_COPY_AND_ASSIGN(CodeEmitterX64Test);
};

std::string CodeEmitterX64Test::Emit(Function* function) {
  TestMachineCodeBuilder builder;
  CodeEmitter emitter(factory(), &builder);
  emitter.Process(function);
  return builder.GetResult();
}

// Test cases...

TEST_F(CodeEmitterX64Test, Basic) {
  auto const function = factory()->NewFunction({});
  EXPECT_EQ("0000 C3\n", Emit(function));
}

TEST_F(CodeEmitterX64Test, Call) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewCallInstruction(NewStringValue8("Foo")));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "string +0001 \"Foo\"\n"
      "0000 E8 00 00 00 00 C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, FrameSlot) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(NewCopyInstruction(Target::GetRegister(isa::RAX),
                                   Value::FrameSlot(Value::Int64Type(), 0)));
  editor.Append(NewCopyInstruction(Value::FrameSlot(Value::Int32Type(), 8),
                                   Target::GetRegister(isa::EDX)));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ("0000 48 8B 05 89 55 08 C3\n", Emit(function));
}

TEST_F(CodeEmitterX64Test, StackSlot) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(NewCopyInstruction(Target::GetRegister(isa::RAX),
                                   Value::StackSlot(Value::Int64Type(), 0)));
  editor.Append(NewCopyInstruction(Value::StackSlot(Value::Int32Type(), 8),
                                   Target::GetRegister(isa::EDX)));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ("0000 48 8B 04 24 89 54 24 08 C3\n", Emit(function));
}

}  // namespace
}  // namespace lir
}  // namespace elang
