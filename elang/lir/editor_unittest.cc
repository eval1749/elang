// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// EditorTest
//
class EditorTest : public testing::LirTest {
 protected:
  EditorTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(EditorTest);
};

// Test cases...

TEST_F(EditorTest, FunctionEmpty) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(EditorTest, FunctionSample1) {
  auto const function = CreateFunctionSample1();
  Editor editor(factory(), function);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  call \"Foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(EditorTest, JumpInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  auto const block = editor.NewBasicBlock(function->exit_block());
  editor.Edit(block);
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));
  editor.Edit(function->entry_block());
  editor.SetJump(block);
  EXPECT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  jmp block3\n"
      "block3:\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(&editor));
}

TEST_F(EditorTest, LiteralInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewLiteralInstruction(
      factory()->NewRegister(),
      factory()->NewIntValue(Value::Size::Size64, 42)));
  editor.Append(factory()->NewLiteralInstruction(
      factory()->NewRegister(), factory()->NewStringValue(L"foo")));
  EXPECT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  mov %r1l = 42l\n"
      "  mov %r2l = \"foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(&editor));
}

}  // namespace lir
}  // namespace elang
