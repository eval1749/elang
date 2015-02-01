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
// LirLiteralsTestX64 offers HIR factories.
//
class LirLiteralsTestX64 : public testing::LirTestX64 {
 protected:
  LirLiteralsTestX64() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirLiteralsTestX64);
};

TEST_F(LirLiteralsTestX64, FunctionEmpty) {
  auto const function = CreateFunctionEmptySample();
  EXPECT_EQ(
      "Function\n"
      "block1:\n"
      "  entry\n"
      "  ret\n"
      "\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirLiteralsTestX64, FunctionSample1) {
  auto const function = CreateFunctionSample1();
  EXPECT_EQ(
      "Function\n"
      "block1:\n"
      "  entry\n"
      "  call \"Foo\"\n"
      "  ret\n"
      "\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

TEST_F(LirLiteralsTestX64, JumpInstruction) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
  auto const block = editor.NewBasicBlock(function->exit_block());
  editor.Edit(block);
  editor.SetReturn();
  editor.Commit();
  editor.Edit(function->entry_block());
  editor.SetJump(block);
  editor.Commit();
  EXPECT_EQ(
      "Function\n"
      "block1:\n"
      "  entry\n"
      "  jmp block3\n"
      "\n"
      "block3:\n"
      "  ret\n"
      "\n"
      "block2:\n"
      "  exit\n",
      FormatFunction(function));
}

}  // namespace lir
}  // namespace elang
