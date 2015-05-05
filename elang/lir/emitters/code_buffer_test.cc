// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/editor.h"
#include "elang/lir/emitters/code_buffer.h"
#include "elang/lir/testing/test_machine_code_builder.h"

namespace elang {
namespace lir {
namespace {

enum Opcode {
  LongBranch = 'B',
  LongJump = 'J',
  Nop = 'N',
  Ret = 'R',
  ShortBranch = 'b',
  ShortJump = 's',
};

typedef CodeBuffer::Jump Jump;

//////////////////////////////////////////////////////////////////////
//
// CodeBufferTest
//
class CodeBufferTest : public testing::LirTest {
 protected:
  CodeBufferTest() = default;
  ~CodeBufferTest() override = default;

  Jump long_branch() { return Jump(LongBranch, 2, 4); }
  Jump short_branch() { return Jump(ShortBranch, 1, 1); }
  Jump long_jump() { return Jump(LongJump, 1, 4); }
  Jump short_jump() { return Jump(ShortJump, 1, 1); }

 private:
  DISALLOW_COPY_AND_ASSIGN(CodeBufferTest);
};

// Test cases...

// entry:
//  jump block2
// block1:
//  ...
// block2:
//  ...
//  br %b, block1, block3
// block3:
//  ret
//
TEST_F(CodeBufferTest, JumpBasic) {
  auto const function = CreateFunctionEmptySample();

  Editor editor(factory(), function);
  auto const block1 = editor.NewBasicBlock(editor.exit_block());
  auto const block2 = editor.NewBasicBlock(editor.exit_block());
  auto const block3 = editor.NewBasicBlock(editor.exit_block());

  CodeBuffer code_buffer(function);

  code_buffer.StartBasicBlock(editor.entry_block());
  code_buffer.EmitJump(long_jump(), short_jump(), block2);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(block1);
  code_buffer.Emit8(Nop);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(block2);
  code_buffer.EmitJump(long_branch(), short_branch(), block1);
  code_buffer.EmitJump(long_jump(), short_jump(), block3);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(block3);
  code_buffer.Emit8(Ret);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(editor.exit_block());
  code_buffer.EndBasicBlock();

  TestMachineCodeBuilder builder;
  code_buffer.Finish(factory(), &builder);
  EXPECT_EQ("0000 73 01 4E 62 FD 73 00 52\n", builder.GetResult());
}

// entry:
//  jump block2
// block1:
//  NOP
// block2:
//  ... NOP x 96 ...
//  br %b, block1, block3
// block3:
//  ret
//
TEST_F(CodeBufferTest, JumpLong) {
  auto const function = CreateFunctionEmptySample();

  Editor editor(factory(), function);
  auto const block1 = editor.NewBasicBlock(editor.exit_block());
  auto const block2 = editor.NewBasicBlock(editor.exit_block());
  auto const block3 = editor.NewBasicBlock(editor.exit_block());

  CodeBuffer code_buffer(function);

  code_buffer.StartBasicBlock(editor.entry_block());
  code_buffer.EmitJump(long_jump(), short_jump(), block2);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(block1);
  code_buffer.Emit8(Nop);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(block2);
  for (auto index = 0; index < 135; ++index)
    code_buffer.Emit8(Nop);
  code_buffer.EmitJump(long_branch(), short_branch(), block1);
  code_buffer.EmitJump(long_jump(), short_jump(), block3);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(block3);
  code_buffer.Emit8(Ret);
  code_buffer.EndBasicBlock();

  code_buffer.StartBasicBlock(editor.exit_block());
  code_buffer.EndBasicBlock();

  TestMachineCodeBuilder builder;
  code_buffer.Finish(factory(), &builder);
  EXPECT_EQ(
      "0000 73 01 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E\n"
      "0010 ... 0x4E x 112 ...\n"
      "0080 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E 00 42 72 FF FF FF\n"
      "0090 73 00 52\n",
      builder.GetResult());
}

}  // namespace
}  // namespace lir
}  // namespace elang
