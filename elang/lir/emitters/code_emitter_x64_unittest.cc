// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/editor.h"
#include "elang/lir/emitters/code_emitter.h"
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

TEST_F(CodeEmitterX64Test, AddInt16) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const ax = Target::GetRegister(isa::AX);
  auto const bx = Target::GetRegister(isa::BX);
  auto const imm16 = Value::SmallInt16(2359);
  auto const imm8 = Value::SmallInt16(42);
  auto const r9w = Target::GetRegister(isa::R9W);
  auto const var33 = Value::FrameSlot(Value::Int16Type(), 33);
  // 66 05 ib ADD AX, imm16
  editor.Append(NewAddInstruction(ax, ax, imm16));
  // 66 81 /0 ib ADD r/m16, imm16
  editor.Append(NewAddInstruction(bx, bx, imm16));
  editor.Append(NewAddInstruction(r9w, r9w, imm16));
  // 66 81 /0 ib ADD r/m16, imm16
  editor.Append(NewAddInstruction(var33, var33, imm16));
  // 66 00 /r ADD r/m16, r16
  editor.Append(NewAddInstruction(bx, bx, ax));
  editor.Append(NewAddInstruction(bx, bx, r9w));
  editor.Append(NewAddInstruction(r9w, r9w, bx));
  editor.Append(NewAddInstruction(var33, var33, bx));
  editor.Append(NewAddInstruction(var33, var33, r9w));
  // 66 02 /r  ADD r16, r/m16
  editor.Append(NewAddInstruction(bx, bx, var33));
  editor.Append(NewAddInstruction(r9w, r9w, var33));
  // 66 83 /0 ib ADD r/m16, imm8
  editor.Append(NewAddInstruction(bx, bx, imm8));
  editor.Append(NewAddInstruction(r9w, r9w, imm8));
  editor.Append(NewAddInstruction(var33, var33, imm8));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 66 05 37 09 66 81 C3 37 09 66 41 81 C1 37 09 66\n"
      "0010 81 45 21 37 09 66 01 C3 66 44 01 CB 66 41 01 D9\n"
      "0020 66 01 5D 21 66 44 01 4D 21 66 03 5D 21 66 44 03\n"
      "0030 4D 21 66 83 C3 2A 66 41 83 C1 2A 66 83 45 21 2A\n"
      "0040 C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, AddInt32) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const eax = Target::GetRegister(isa::EAX);
  auto const ebx = Target::GetRegister(isa::EBX);
  auto const imm32 = Value::SmallInt32(2359);
  auto const imm8 = Value::SmallInt32(42);
  auto const r9d = Target::GetRegister(isa::R9D);
  auto const var33 = Value::FrameSlot(Value::Int32Type(), 33);
  // 05 ib ADD EAX, imm32
  editor.Append(NewAddInstruction(eax, eax, imm32));
  // 81 /0 ib ADD r/m32, imm32
  editor.Append(NewAddInstruction(ebx, ebx, imm32));
  editor.Append(NewAddInstruction(r9d, r9d, imm32));
  // 81 /0 ib ADD r/m32, imm32
  editor.Append(NewAddInstruction(var33, var33, imm32));
  // 00 /r ADD r/m32, r32
  editor.Append(NewAddInstruction(ebx, ebx, eax));
  editor.Append(NewAddInstruction(ebx, ebx, r9d));
  editor.Append(NewAddInstruction(r9d, r9d, ebx));
  editor.Append(NewAddInstruction(var33, var33, ebx));
  editor.Append(NewAddInstruction(var33, var33, r9d));
  // 02 /r  ADD r32, r/m32
  editor.Append(NewAddInstruction(ebx, ebx, var33));
  editor.Append(NewAddInstruction(r9d, r9d, var33));
  // 83 /0 ib ADD r/m32, imm8
  editor.Append(NewAddInstruction(ebx, ebx, imm8));
  editor.Append(NewAddInstruction(r9d, r9d, imm8));
  editor.Append(NewAddInstruction(var33, var33, imm8));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 05 37 09 00 00 81 C3 37 09 00 00 41 81 C1 37 09\n"
      "0010 00 00 81 45 21 37 09 00 00 01 C3 44 01 CB 41 01\n"
      "0020 D9 01 5D 21 44 01 4D 21 03 5D 21 44 03 4D 21 83\n"
      "0030 C3 2A 41 83 C1 2A 83 45 21 2A C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, AddInt64) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const rax = Target::GetRegister(isa::RAX);
  auto const rbx = Target::GetRegister(isa::RBX);
  auto const imm32 = Value::SmallInt64(2359);
  auto const imm8 = Value::SmallInt64(42);
  auto const r9 = Target::GetRegister(isa::R9);
  auto const var33 = Value::FrameSlot(Value::Int64Type(), 33);
  // REX.W 05 ib ADD RAX, imm32
  editor.Append(NewAddInstruction(rax, rax, imm32));
  // REX.W 81 /0 ib ADD r/m64, imm32
  editor.Append(NewAddInstruction(rbx, rbx, imm32));
  editor.Append(NewAddInstruction(r9, r9, imm32));
  // REX.W 81 /0 ib ADD r/m64, imm32
  editor.Append(NewAddInstruction(var33, var33, imm32));
  // REX.W 00 /r ADD r/m64, r64
  editor.Append(NewAddInstruction(rbx, rbx, rax));
  editor.Append(NewAddInstruction(rbx, rbx, r9));
  editor.Append(NewAddInstruction(r9, r9, rbx));
  editor.Append(NewAddInstruction(var33, var33, rbx));
  editor.Append(NewAddInstruction(var33, var33, r9));
  // REX.W 02 /r  ADD r64, r/m64
  editor.Append(NewAddInstruction(rbx, rbx, var33));
  editor.Append(NewAddInstruction(r9, r9, var33));
  // REX.W 83 /0 ib ADD r/m64, imm8
  editor.Append(NewAddInstruction(rbx, rbx, imm8));
  editor.Append(NewAddInstruction(r9, r9, imm8));
  editor.Append(NewAddInstruction(var33, var33, imm8));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 48 05 37 09 00 00 48 81 C3 37 09 00 00 49 81 C1\n"
      "0010 37 09 00 00 48 81 45 21 37 09 00 00 48 01 C3 4C\n"
      "0020 01 CB 49 01 D9 48 01 5D 21 4C 01 4D 21 48 03 5D\n"
      "0030 21 4C 03 4D 21 48 83 C3 2A 49 83 C1 2A 48 83 45\n"
      "0040 21 2A C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, AddInt8) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const al = Target::GetRegister(isa::AL);
  auto const bl = Target::GetRegister(isa::BL);
  auto const cl = Target::GetRegister(isa::CL);
  auto const dil = Target::GetRegister(isa::DIL);
  auto const imm8 = Value::SmallInt8(42);
  auto const r9b = Target::GetRegister(isa::R9B);
  auto const var33 = Value::FrameSlot(Value::Int8Type(), 33);
  // 04 ib ADD AL, imm8
  editor.Append(NewAddInstruction(al, al, imm8));
  // 80 /0 ib ADD r/m8, imm8
  editor.Append(NewAddInstruction(bl, bl, imm8));
  editor.Append(NewAddInstruction(dil, dil, imm8));
  editor.Append(NewAddInstruction(r9b, r9b, imm8));
  editor.Append(NewAddInstruction(var33, var33, imm8));
  //  00 /r ADD r/m8, r8
  editor.Append(NewAddInstruction(var33, var33, bl));
  editor.Append(NewAddInstruction(var33, var33, dil));
  editor.Append(NewAddInstruction(var33, var33, r9b));
  // 02 /r  ADD r8, r/m8
  editor.Append(NewAddInstruction(bl, bl, var33));
  editor.Append(NewAddInstruction(dil, dil, var33));
  editor.Append(NewAddInstruction(r9b, r9b, var33));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 04 2A 80 C3 2A 40 80 C7 2A 41 80 C1 2A 80 45 21\n"
      "0010 2A 00 5D 21 40 00 7D 21 44 00 4D 21 02 5D 21 40\n"
      "0020 02 7D 21 44 02 4D 21 C3\n",
      Emit(function));
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

TEST_F(CodeEmitterX64Test, Empty) {
  auto const function = factory()->NewFunction({});
  EXPECT_EQ("0000 C3\n", Emit(function));
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

TEST_F(CodeEmitterX64Test, Int8) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(NewLiteralInstruction(Target::GetRegister(isa::CL),
                                      Value::SmallInt8(42)));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ("0000 B1 2A C3\n", Emit(function));
}

TEST_F(CodeEmitterX64Test, Int16) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(NewLiteralInstruction(Target::GetRegister(isa::AX),
                                      Value::SmallInt16(42)));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ("0000 66 B8 2A 00 C3\n", Emit(function));
}

TEST_F(CodeEmitterX64Test, Int32) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(NewLiteralInstruction(Target::GetRegister(isa::EAX),
                                      Value::SmallInt32(42)));
  editor.Append(NewLiteralInstruction(
      Target::GetRegister(isa::EAX), NewIntValue(Value::Int32Type(), 1 << 30)));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ("0000 B8 2A 00 00 00 B8 00 00 00 40 C3\n", Emit(function));
}

TEST_F(CodeEmitterX64Test, Int64) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(NewLiteralInstruction(Target::GetRegister(isa::RAX),
                                      Value::SmallInt64(42)));
  editor.Append(NewLiteralInstruction(Target::GetRegister(isa::RAX),
                                      Value::SmallInt64(-42)));
  editor.Append(NewLiteralInstruction(
      Target::GetRegister(isa::RAX), NewIntValue(Value::Int64Type(), 1 << 30)));
  editor.Append(
      NewLiteralInstruction(Target::GetRegister(isa::RAX),
                            NewIntValue(Value::Int64Type(), -1 << 30)));
  editor.Append(NewLiteralInstruction(
      Target::GetRegister(isa::RAX),
      NewIntValue(Value::Int64Type(), static_cast<int64_t>(1) << 42)));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 B8 2A 00 00 00 48 C7 C0 D6 FF FF FF B8 00 00 00\n"
      "0010 40 48 B8 00 00 00 C0 FF FF FF FF 48 B8 00 00 00\n"
      "0020 00 00 04 00 00 C3\n",
      Emit(function));
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
