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

TEST_F(CodeEmitterX64Test, CmpInt32) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const cond = NewConditional();
  auto const eax = Target::GetRegister(isa::EAX);
  auto const ebx = Target::GetRegister(isa::EBX);
  auto const eq = IntegerCondition::Equal;
  auto const imm32 = Value::SmallInt32(2359);
  auto const imm8 = Value::SmallInt32(42);
  auto const r9d = Target::GetRegister(isa::R9D);
  auto const var33 = Value::FrameSlot(Value::Int32Type(), 33);
  // 05 ib CMP EAX, imm32
  editor.Append(NewCmpInstruction(cond, eq, eax, imm32));
  // 81 /0 ib CMP r/m32, imm32
  editor.Append(NewCmpInstruction(cond, eq, ebx, imm32));
  editor.Append(NewCmpInstruction(cond, eq, r9d, imm32));
  // 81 /0 ib CMP r/m32, imm32
  editor.Append(NewCmpInstruction(cond, eq, var33, imm32));
  // 00 /r CMP r/m32, r32
  editor.Append(NewCmpInstruction(cond, eq, ebx, eax));
  editor.Append(NewCmpInstruction(cond, eq, ebx, r9d));
  editor.Append(NewCmpInstruction(cond, eq, r9d, ebx));
  editor.Append(NewCmpInstruction(cond, eq, var33, ebx));
  editor.Append(NewCmpInstruction(cond, eq, var33, r9d));
  // 02 /r  CMP r32, r/m32
  editor.Append(NewCmpInstruction(cond, eq, ebx, var33));
  editor.Append(NewCmpInstruction(cond, eq, r9d, var33));
  // 83 /0 ib CMP r/m32, imm8
  editor.Append(NewCmpInstruction(cond, eq, ebx, imm8));
  editor.Append(NewCmpInstruction(cond, eq, r9d, imm8));
  editor.Append(NewCmpInstruction(cond, eq, var33, imm8));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 3D 37 09 00 00 81 FB 37 09 00 00 41 81 F9 37 09\n"
      "0010 00 00 81 7D 21 37 09 00 00 39 C3 44 39 CB 41 39\n"
      "0020 D9 39 5D 21 44 39 4D 21 3B 5D 21 44 3B 4D 21 83\n"
      "0030 FB 2A 41 83 F9 2A 83 7D 21 2A C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, CopyInt16) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const ax = Target::GetRegister(isa::AX);
  auto const bx = Target::GetRegister(isa::BX);
  auto const di = Target::GetRegister(isa::DI);
  auto const r8w = Target::GetRegister(isa::R8W);
  auto const r9w = Target::GetRegister(isa::R9W);
  auto const var33 = Value::FrameSlot(Value::Int16Type(), 33);

  editor.Append(NewCopyInstruction(ax, bx));
  editor.Append(NewCopyInstruction(ax, di));
  editor.Append(NewCopyInstruction(ax, r8w));
  editor.Append(NewCopyInstruction(ax, r9w));
  editor.Append(NewCopyInstruction(ax, var33));

  editor.Append(NewCopyInstruction(bx, ax));
  editor.Append(NewCopyInstruction(bx, di));
  editor.Append(NewCopyInstruction(bx, r8w));
  editor.Append(NewCopyInstruction(bx, r9w));
  editor.Append(NewCopyInstruction(bx, var33));

  editor.Append(NewCopyInstruction(di, ax));
  editor.Append(NewCopyInstruction(di, bx));
  editor.Append(NewCopyInstruction(di, r8w));
  editor.Append(NewCopyInstruction(di, r9w));
  editor.Append(NewCopyInstruction(di, var33));

  editor.Append(NewCopyInstruction(r8w, ax));
  editor.Append(NewCopyInstruction(r8w, bx));
  editor.Append(NewCopyInstruction(r8w, di));
  editor.Append(NewCopyInstruction(r8w, r9w));
  editor.Append(NewCopyInstruction(r8w, var33));

  editor.Append(NewCopyInstruction(r9w, ax));
  editor.Append(NewCopyInstruction(r9w, bx));
  editor.Append(NewCopyInstruction(r9w, di));
  editor.Append(NewCopyInstruction(r9w, r8w));
  editor.Append(NewCopyInstruction(r9w, var33));

  editor.Append(NewCopyInstruction(var33, ax));
  editor.Append(NewCopyInstruction(var33, bx));
  editor.Append(NewCopyInstruction(var33, di));
  editor.Append(NewCopyInstruction(var33, r8w));
  editor.Append(NewCopyInstruction(var33, r9w));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 66 8B C3 66 8B C7 66 41 8B C0 66 41 8B C1 66 8B\n"
      "0010 45 21 66 8B D8 66 8B DF 66 41 8B D8 66 41 8B D9\n"
      "0020 66 8B 5D 21 66 8B F8 66 8B FB 66 41 8B F8 66 41\n"
      "0030 8B F9 66 8B 7D 21 66 44 8B C0 66 44 8B C3 66 44\n"
      "0040 8B C7 66 45 8B C1 66 44 8B 45 21 66 44 8B C8 66\n"
      "0050 44 8B CB 66 44 8B CF 66 45 8B C8 66 44 8B 4D 21\n"
      "0060 66 89 45 21 66 89 5D 21 66 89 7D 21 66 44 89 45\n"
      "0070 21 66 44 89 4D 21 C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, CopyInt32) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const eax = Target::GetRegister(isa::EAX);
  auto const ebx = Target::GetRegister(isa::EBX);
  auto const edi = Target::GetRegister(isa::EDI);
  auto const r8d = Target::GetRegister(isa::R8D);
  auto const r9d = Target::GetRegister(isa::R9D);
  auto const var33 = Value::FrameSlot(Value::Int32Type(), 33);

  editor.Append(NewCopyInstruction(eax, ebx));
  editor.Append(NewCopyInstruction(eax, edi));
  editor.Append(NewCopyInstruction(eax, r8d));
  editor.Append(NewCopyInstruction(eax, r9d));
  editor.Append(NewCopyInstruction(eax, var33));

  editor.Append(NewCopyInstruction(ebx, eax));
  editor.Append(NewCopyInstruction(ebx, edi));
  editor.Append(NewCopyInstruction(ebx, r8d));
  editor.Append(NewCopyInstruction(ebx, r9d));
  editor.Append(NewCopyInstruction(ebx, var33));

  editor.Append(NewCopyInstruction(edi, eax));
  editor.Append(NewCopyInstruction(edi, ebx));
  editor.Append(NewCopyInstruction(edi, r8d));
  editor.Append(NewCopyInstruction(edi, r9d));
  editor.Append(NewCopyInstruction(edi, var33));

  editor.Append(NewCopyInstruction(r8d, eax));
  editor.Append(NewCopyInstruction(r8d, ebx));
  editor.Append(NewCopyInstruction(r8d, edi));
  editor.Append(NewCopyInstruction(r8d, r9d));
  editor.Append(NewCopyInstruction(r8d, var33));

  editor.Append(NewCopyInstruction(r9d, eax));
  editor.Append(NewCopyInstruction(r9d, ebx));
  editor.Append(NewCopyInstruction(r9d, edi));
  editor.Append(NewCopyInstruction(r9d, r8d));
  editor.Append(NewCopyInstruction(r9d, var33));

  editor.Append(NewCopyInstruction(var33, eax));
  editor.Append(NewCopyInstruction(var33, ebx));
  editor.Append(NewCopyInstruction(var33, edi));
  editor.Append(NewCopyInstruction(var33, r8d));
  editor.Append(NewCopyInstruction(var33, r9d));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 8B C3 8B C7 41 8B C0 41 8B C1 8B 45 21 8B D8 8B\n"
      "0010 DF 41 8B D8 41 8B D9 8B 5D 21 8B F8 8B FB 41 8B\n"
      "0020 F8 41 8B F9 8B 7D 21 44 8B C0 44 8B C3 44 8B C7\n"
      "0030 45 8B C1 44 8B 45 21 44 8B C8 44 8B CB 44 8B CF\n"
      "0040 45 8B C8 44 8B 4D 21 89 45 21 89 5D 21 89 7D 21\n"
      "0050 44 89 45 21 44 89 4D 21 C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, CopyInt64) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const rax = Target::GetRegister(isa::RAX);
  auto const rbx = Target::GetRegister(isa::RBX);
  auto const rdi = Target::GetRegister(isa::RDI);
  auto const r8 = Target::GetRegister(isa::R8);
  auto const r9 = Target::GetRegister(isa::R9);
  auto const var33 = Value::FrameSlot(Value::Int64Type(), 33);

  editor.Append(NewCopyInstruction(rax, rbx));
  editor.Append(NewCopyInstruction(rax, rdi));
  editor.Append(NewCopyInstruction(rax, r8));
  editor.Append(NewCopyInstruction(rax, r9));
  editor.Append(NewCopyInstruction(rax, var33));

  editor.Append(NewCopyInstruction(rbx, rax));
  editor.Append(NewCopyInstruction(rbx, rdi));
  editor.Append(NewCopyInstruction(rbx, r8));
  editor.Append(NewCopyInstruction(rbx, r9));
  editor.Append(NewCopyInstruction(rbx, var33));

  editor.Append(NewCopyInstruction(rdi, rax));
  editor.Append(NewCopyInstruction(rdi, rbx));
  editor.Append(NewCopyInstruction(rdi, r8));
  editor.Append(NewCopyInstruction(rdi, r9));
  editor.Append(NewCopyInstruction(rdi, var33));

  editor.Append(NewCopyInstruction(r8, rax));
  editor.Append(NewCopyInstruction(r8, rbx));
  editor.Append(NewCopyInstruction(r8, rdi));
  editor.Append(NewCopyInstruction(r8, r9));
  editor.Append(NewCopyInstruction(r8, var33));

  editor.Append(NewCopyInstruction(r9, rax));
  editor.Append(NewCopyInstruction(r9, rbx));
  editor.Append(NewCopyInstruction(r9, rdi));
  editor.Append(NewCopyInstruction(r9, r8));
  editor.Append(NewCopyInstruction(r9, var33));

  editor.Append(NewCopyInstruction(var33, rax));
  editor.Append(NewCopyInstruction(var33, rbx));
  editor.Append(NewCopyInstruction(var33, rdi));
  editor.Append(NewCopyInstruction(var33, r8));
  editor.Append(NewCopyInstruction(var33, r9));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 48 8B C3 48 8B C7 49 8B C0 49 8B C1 48 8B 45 21\n"
      "0010 48 8B D8 48 8B DF 49 8B D8 49 8B D9 48 8B 5D 21\n"
      "0020 48 8B F8 48 8B FB 49 8B F8 49 8B F9 48 8B 7D 21\n"
      "0030 4C 8B C0 4C 8B C3 4C 8B C7 4D 8B C1 4C 8B 45 21\n"
      "0040 4C 8B C8 4C 8B CB 4C 8B CF 4D 8B C8 4C 8B 4D 21\n"
      "0050 48 89 45 21 48 89 5D 21 48 89 7D 21 4C 89 45 21\n"
      "0060 4C 89 4D 21 C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, CopyInt8) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const al = Target::GetRegister(isa::AL);
  auto const bl = Target::GetRegister(isa::BL);
  auto const dil = Target::GetRegister(isa::DIL);
  auto const r8b = Target::GetRegister(isa::R8B);
  auto const r9b = Target::GetRegister(isa::R9B);
  auto const var33 = Value::FrameSlot(Value::Int8Type(), 33);

  editor.Append(NewCopyInstruction(al, bl));
  editor.Append(NewCopyInstruction(al, dil));
  editor.Append(NewCopyInstruction(al, r8b));
  editor.Append(NewCopyInstruction(al, r9b));
  editor.Append(NewCopyInstruction(al, var33));

  editor.Append(NewCopyInstruction(bl, al));
  editor.Append(NewCopyInstruction(bl, dil));
  editor.Append(NewCopyInstruction(bl, r8b));
  editor.Append(NewCopyInstruction(bl, r9b));
  editor.Append(NewCopyInstruction(bl, var33));

  editor.Append(NewCopyInstruction(dil, al));
  editor.Append(NewCopyInstruction(dil, bl));
  editor.Append(NewCopyInstruction(dil, r8b));
  editor.Append(NewCopyInstruction(dil, r9b));
  editor.Append(NewCopyInstruction(dil, var33));

  editor.Append(NewCopyInstruction(r8b, al));
  editor.Append(NewCopyInstruction(r8b, bl));
  editor.Append(NewCopyInstruction(r8b, dil));
  editor.Append(NewCopyInstruction(r8b, r9b));
  editor.Append(NewCopyInstruction(r8b, var33));

  editor.Append(NewCopyInstruction(r9b, al));
  editor.Append(NewCopyInstruction(r9b, bl));
  editor.Append(NewCopyInstruction(r9b, dil));
  editor.Append(NewCopyInstruction(r9b, r8b));
  editor.Append(NewCopyInstruction(r9b, var33));

  editor.Append(NewCopyInstruction(var33, al));
  editor.Append(NewCopyInstruction(var33, bl));
  editor.Append(NewCopyInstruction(var33, dil));
  editor.Append(NewCopyInstruction(var33, r8b));
  editor.Append(NewCopyInstruction(var33, r9b));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 8A C3 40 8A C7 41 8A C0 41 8A C1 8A 45 21 8A D8\n"
      "0010 40 8A DF 41 8A D8 41 8A D9 8A 5D 21 40 8A F8 40\n"
      "0020 8A FB 41 8A F8 41 8A F9 40 8A 7D 21 44 8A C0 44\n"
      "0030 8A C3 44 8A C7 45 8A C1 44 8A 45 21 44 8A C8 44\n"
      "0040 8A CB 44 8A CF 45 8A C8 44 8A 4D 21 88 45 21 88\n"
      "0050 5D 21 40 88 7D 21 44 88 45 21 44 88 4D 21 C3\n",
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

TEST_F(CodeEmitterX64Test, LiteralInt16) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const ax = Target::GetRegister(isa::AX);
  auto const bx = Target::GetRegister(isa::BX);
  auto const di = Target::GetRegister(isa::DI);
  auto const imm16 = Value::SmallInt16(42);
  auto const r9w = Target::GetRegister(isa::R9W);
  auto const var33 = Value::FrameSlot(Value::Int16Type(), 33);
  editor.Append(NewLiteralInstruction(ax, imm16));
  editor.Append(NewLiteralInstruction(bx, imm16));
  editor.Append(NewLiteralInstruction(di, imm16));
  editor.Append(NewLiteralInstruction(r9w, imm16));
  editor.Append(NewLiteralInstruction(var33, imm16));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 66 B8 2A 00 66 BB 2A 00 66 BF 2A 00 66 41 B9 2A\n"
      "0010 00 66 C7 45 21 2A 00 C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, LiteralInt32) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const eax = Target::GetRegister(isa::EAX);
  auto const ebx = Target::GetRegister(isa::EBX);
  auto const edi = Target::GetRegister(isa::EDI);
  auto const imm32 = Value::SmallInt32(42);
  auto const imm32x = NewIntValue(Value::Int32Type(), 0x77665544);
  auto const r9d = Target::GetRegister(isa::R9D);
  auto const var33 = Value::FrameSlot(Value::Int32Type(), 33);
  editor.Append(NewLiteralInstruction(eax, imm32));
  editor.Append(NewLiteralInstruction(eax, imm32x));

  editor.Append(NewLiteralInstruction(ebx, imm32));
  editor.Append(NewLiteralInstruction(ebx, imm32x));

  editor.Append(NewLiteralInstruction(edi, imm32));
  editor.Append(NewLiteralInstruction(edi, imm32x));

  editor.Append(NewLiteralInstruction(r9d, imm32));
  editor.Append(NewLiteralInstruction(r9d, imm32x));

  editor.Append(NewLiteralInstruction(var33, imm32));
  editor.Append(NewLiteralInstruction(var33, imm32x));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 B8 2A 00 00 00 B8 44 55 66 77 BB 2A 00 00 00 BB\n"
      "0010 44 55 66 77 BF 2A 00 00 00 BF 44 55 66 77 41 B9\n"
      "0020 2A 00 00 00 41 B9 44 55 66 77 C7 45 21 2A 00 00\n"
      "0030 00 C7 45 21 44 55 66 77 C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, LiteralInt64) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const rax = Target::GetRegister(isa::RAX);
  auto const rbx = Target::GetRegister(isa::RBX);
  auto const rdi = Target::GetRegister(isa::RDI);
  auto const imm32 = Value::SmallInt64(42);
  auto const imm32x = NewIntValue(Value::Int64Type(), 0x77665544);
  auto const imm64 = Value::SmallInt64(42);
  auto const imm64x = NewIntValue(Value::Int64Type(), 0x7766554433221100ll);
  auto const minus64 = Value::SmallInt64(-42);
  auto const minus64x = NewIntValue(Value::Int64Type(), -0x7766554433221100ll);
  auto const r9 = Target::GetRegister(isa::R9);
  auto const var33 = Value::FrameSlot(Value::Int64Type(), 33);

  editor.Append(NewLiteralInstruction(rax, imm32));
  editor.Append(NewLiteralInstruction(rax, imm32x));
  editor.Append(NewLiteralInstruction(rax, imm64));
  editor.Append(NewLiteralInstruction(rax, imm64x));
  editor.Append(NewLiteralInstruction(rax, minus64));
  editor.Append(NewLiteralInstruction(rax, minus64x));

  editor.Append(NewLiteralInstruction(rbx, imm32));
  editor.Append(NewLiteralInstruction(rbx, imm32x));
  editor.Append(NewLiteralInstruction(rbx, imm64));
  editor.Append(NewLiteralInstruction(rbx, imm64x));
  editor.Append(NewLiteralInstruction(rbx, minus64));
  editor.Append(NewLiteralInstruction(rbx, minus64x));

  editor.Append(NewLiteralInstruction(r9, imm32));
  editor.Append(NewLiteralInstruction(r9, imm32x));
  editor.Append(NewLiteralInstruction(r9, imm64));
  editor.Append(NewLiteralInstruction(r9, imm64x));
  editor.Append(NewLiteralInstruction(r9, minus64));
  editor.Append(NewLiteralInstruction(r9, minus64x));

  editor.Append(NewLiteralInstruction(var33, imm32));
  editor.Append(NewLiteralInstruction(var33, imm32x));
  editor.Append(NewLiteralInstruction(var33, imm64));
  editor.Append(NewLiteralInstruction(var33, minus64));
  // Note: There are no instruction to store 64-bit integer to 64-bit memory.
  // So, we can't use |LiteralInstruction| with |var33| and |imm64x|.
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 B8 2A 00 00 00 B8 44 55 66 77 B8 2A 00 00 00 48\n"
      "0010 B8 00 11 22 33 44 55 66 77 C7 C0 D6 FF FF FF 48\n"
      "0020 B8 00 EF DD CC BB AA 99 88 BB 2A 00 00 00 BB 44\n"
      "0030 55 66 77 BB 2A 00 00 00 48 BB 00 11 22 33 44 55\n"
      "0040 66 77 C7 C3 D6 FF FF FF 48 BB 00 EF DD CC BB AA\n"
      "0050 99 88 41 B9 2A 00 00 00 41 B9 44 55 66 77 41 B9\n"
      "0060 2A 00 00 00 49 B9 00 11 22 33 44 55 66 77 41 C7\n"
      "0070 C1 D6 FF FF FF 49 B9 00 EF DD CC BB AA 99 88 C7\n"
      "0080 45 21 2A 00 00 00 C7 45 21 44 55 66 77 C7 45 21\n"
      "0090 2A 00 00 00 C7 45 21 D6 FF FF FF C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, LiteralInt8) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const al = Target::GetRegister(isa::AL);
  auto const bl = Target::GetRegister(isa::BL);
  auto const dil = Target::GetRegister(isa::DIL);
  auto const imm8 = Value::SmallInt8(42);
  auto const r9b = Target::GetRegister(isa::R9B);
  auto const var33 = Value::FrameSlot(Value::Int8Type(), 33);
  editor.Append(NewLiteralInstruction(al, imm8));
  editor.Append(NewLiteralInstruction(bl, imm8));
  editor.Append(NewLiteralInstruction(dil, imm8));
  editor.Append(NewLiteralInstruction(r9b, imm8));
  editor.Append(NewLiteralInstruction(var33, imm8));
  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ("0000 B0 2A B3 2A 40 B7 2A 41 B1 2A C6 45 21 2A C3\n",
            Emit(function));
}

TEST_F(CodeEmitterX64Test, ShlInt16) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const cl = Target::GetRegister(isa::CL);
  auto const ax = Target::GetRegister(isa::AX);
  auto const bx = Target::GetRegister(isa::BX);
  auto const imm32 = Value::SmallInt32(42);
  auto const one = Value::SmallInt32(1);
  auto const r9w = Target::GetRegister(isa::R9W);
  auto const var33 = Value::FrameSlot(Value::Int16Type(), 33);

  editor.Append(NewShlInstruction(ax, ax, one));
  editor.Append(NewShlInstruction(ax, ax, cl));
  editor.Append(NewShlInstruction(ax, ax, imm32));

  editor.Append(NewShlInstruction(bx, bx, one));
  editor.Append(NewShlInstruction(bx, bx, cl));
  editor.Append(NewShlInstruction(bx, bx, imm32));

  editor.Append(NewShlInstruction(r9w, r9w, one));
  editor.Append(NewShlInstruction(r9w, r9w, cl));
  editor.Append(NewShlInstruction(r9w, r9w, imm32));

  editor.Append(NewShlInstruction(var33, var33, one));
  editor.Append(NewShlInstruction(var33, var33, cl));
  editor.Append(NewShlInstruction(var33, var33, imm32));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 66 D1 E0 66 D3 E0 66 C1 E0 2A 66 D1 E3 66 D3 E3\n"
      "0010 66 C1 E3 2A 66 41 D1 E1 66 41 D3 E1 66 41 C1 E1\n"
      "0020 2A 66 D1 65 21 66 D3 65 21 66 C1 65 21 2A C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, ShlInt32) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const cl = Target::GetRegister(isa::CL);
  auto const eax = Target::GetRegister(isa::EAX);
  auto const ebx = Target::GetRegister(isa::EBX);
  auto const edi = Target::GetRegister(isa::EDI);
  auto const imm32 = Value::SmallInt32(42);
  auto const one = Value::SmallInt32(1);
  auto const r9d = Target::GetRegister(isa::R9D);
  auto const var33 = Value::FrameSlot(Value::Int32Type(), 33);

  editor.Append(NewShlInstruction(eax, eax, one));
  editor.Append(NewShlInstruction(eax, eax, cl));
  editor.Append(NewShlInstruction(eax, eax, imm32));

  editor.Append(NewShlInstruction(ebx, ebx, one));
  editor.Append(NewShlInstruction(ebx, ebx, cl));
  editor.Append(NewShlInstruction(ebx, ebx, imm32));

  editor.Append(NewShlInstruction(r9d, r9d, one));
  editor.Append(NewShlInstruction(r9d, r9d, cl));
  editor.Append(NewShlInstruction(r9d, r9d, imm32));

  editor.Append(NewShlInstruction(var33, var33, one));
  editor.Append(NewShlInstruction(var33, var33, cl));
  editor.Append(NewShlInstruction(var33, var33, imm32));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 D1 E0 D3 E0 C1 E0 2A D1 E3 D3 E3 C1 E3 2A 41 D1\n"
      "0010 E1 41 D3 E1 41 C1 E1 2A D1 65 21 D3 65 21 C1 65\n"
      "0020 21 2A C3\n",
      Emit(function));
}

TEST_F(CodeEmitterX64Test, ShrInt8) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const cl = Target::GetRegister(isa::CL);
  auto const al = Target::GetRegister(isa::AL);
  auto const bl = Target::GetRegister(isa::BL);
  auto const dil = Target::GetRegister(isa::DIL);
  auto const imm32 = Value::SmallInt32(42);
  auto const one = Value::SmallInt32(1);
  auto const r9b = Target::GetRegister(isa::R9B);
  auto const var33 = Value::FrameSlot(Value::Int8Type(), 33);

  editor.Append(NewShrInstruction(al, al, one));
  editor.Append(NewShrInstruction(al, al, cl));
  editor.Append(NewShrInstruction(al, al, imm32));

  editor.Append(NewShrInstruction(bl, bl, one));
  editor.Append(NewShrInstruction(bl, bl, cl));
  editor.Append(NewShrInstruction(bl, bl, imm32));

  editor.Append(NewShrInstruction(dil, dil, one));
  editor.Append(NewShrInstruction(dil, dil, cl));
  editor.Append(NewShrInstruction(dil, dil, imm32));

  editor.Append(NewShrInstruction(r9b, r9b, one));
  editor.Append(NewShrInstruction(r9b, r9b, cl));
  editor.Append(NewShrInstruction(r9b, r9b, imm32));

  editor.Append(NewShrInstruction(var33, var33, one));
  editor.Append(NewShrInstruction(var33, var33, cl));
  editor.Append(NewShrInstruction(var33, var33, imm32));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 D0 F8 D2 F8 C0 F8 2A D0 FB D2 FB C0 FB 2A 40 D0\n"
      "0010 FF 40 D2 FF 40 C0 FF 2A 41 D0 F9 41 D2 F9 41 C0\n"
      "0020 F9 2A D0 7D 21 D2 7D 21 C0 7D 21 2A C3\n",
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

TEST_F(CodeEmitterX64Test, UShrInt64) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const cl = Target::GetRegister(isa::CL);
  auto const rax = Target::GetRegister(isa::RAX);
  auto const rbx = Target::GetRegister(isa::RBX);
  auto const imm32 = Value::SmallInt32(42);
  auto const one = Value::SmallInt32(1);
  auto const r9 = Target::GetRegister(isa::R9);
  auto const var33 = Value::FrameSlot(Value::Int64Type(), 33);

  editor.Append(NewUShrInstruction(rax, rax, one));
  editor.Append(NewUShrInstruction(rax, rax, cl));
  editor.Append(NewUShrInstruction(rax, rax, imm32));

  editor.Append(NewUShrInstruction(rbx, rbx, one));
  editor.Append(NewUShrInstruction(rbx, rbx, cl));
  editor.Append(NewUShrInstruction(rbx, rbx, imm32));

  editor.Append(NewUShrInstruction(r9, r9, one));
  editor.Append(NewUShrInstruction(r9, r9, cl));
  editor.Append(NewUShrInstruction(r9, r9, imm32));

  editor.Append(NewUShrInstruction(var33, var33, one));
  editor.Append(NewUShrInstruction(var33, var33, cl));
  editor.Append(NewUShrInstruction(var33, var33, imm32));

  ASSERT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "0000 48 D1 E8 48 D3 E8 48 C1 E8 2A 48 D1 EB 48 D3 EB\n"
      "0010 48 C1 EB 2A 49 D1 E9 49 D3 E9 49 C1 E9 2A 48 D1\n"
      "0020 6D 21 48 D3 6D 21 48 C1 6D 21 2A C3\n",
      Emit(function));
}

}  // namespace
}  // namespace lir
}  // namespace elang
