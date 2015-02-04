// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/cg/testing/cg_test.h"

#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/values.h"

namespace elang {
namespace cg {
namespace {

//////////////////////////////////////////////////////////////////////
//
// GeneratorX64Test
//
class GeneratorX64Test : public testing::CgTest {
 protected:
  GeneratorX64Test() = default;
  ~GeneratorX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(GeneratorX64Test);
};

// Test cases...

TEST_F(GeneratorX64Test, Basic) {
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Generate(function()));
}

TEST_F(GeneratorX64Test, BinaryOperation) {
  auto const function = NewFunction(
      int32_type(), types()->NewTupleType({int32_type(), int32_type()}));
  auto const entry = function->entry_block()->first_instruction();
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const param0 = factory()->NewGetInstruction(entry, 0);
  editor.Append(param0);
  auto const param1 = factory()->NewGetInstruction(entry, 1);
  editor.Append(param1);
  auto const add_instr =
      factory()->NewAddInstruction(int32_type(), param0, param1);
  editor.Append(add_instr);
  editor.SetReturn(add_instr);
  editor.Commit();
  ASSERT_TRUE(editor.Validate());

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  pcopy %r1, %r2 = ECX, EDX\n"
      "  add %r3 = %r1, %r2\n"
      "  mov EAX = %r3\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Generate(function));
}

TEST_F(GeneratorX64Test, Parameter) {
  auto const function =
      NewFunction(void_type(), types()->NewTupleType({int32_type(),
                                                      int64_type(),
                                                      bool_type(),
                                                      float64_type(),
                                                      int64_type()}));
  auto const entry = function->entry_block()->first_instruction();
  auto const num_parameters = entry->type()->as<hir::TupleType>()->size();
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  for (auto position = 0; position < num_parameters; ++position)
    editor.Append(factory()->NewGetInstruction(entry, position));
  editor.Commit();
  ASSERT_TRUE(editor.Validate());

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  pcopy %r1, %r2, %r3, %f1, %r4 = ECX, RDX, R8D, XMM3, %param[4]\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Generate(function));
}

TEST_F(GeneratorX64Test, ReturnInt32) {
  auto const function = NewFunction(int32_type(), void_type());
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.SetReturn(factory()->NewInt32Literal(42));
  editor.Commit();
  ASSERT_TRUE(editor.Validate());
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  mov EAX = 42\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Generate(function));
}

TEST_F(GeneratorX64Test, ReturnInt64) {
  auto const function = NewFunction(int64_type(), void_type());
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.SetReturn(factory()->NewInt64Literal(42));
  editor.Commit();
  ASSERT_TRUE(editor.Validate());
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  mov RAX = 42\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Generate(function));
}

}  // namespace
}  // namespace cg
}  // namespace elang
