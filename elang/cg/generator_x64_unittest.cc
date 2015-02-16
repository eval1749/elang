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
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
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
  ASSERT_EQ("", Validate(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  pcopy %r1, %r2 = ECX, EDX\n"
      "  add %r3 = %r1, %r2\n"
      "  mov EAX = %r3\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Generate(function));
}

TEST_F(GeneratorX64Test, Call) {
  hir::Editor editor(factory(), function());
  editor.Edit(function()->entry_block());
  auto const params_type = types()->NewTupleType({int32_type(),
                                                  bool_type(),
                                                  int64_type(),
                                                  int16_type(),
                                                  float32_type(),
                                                  float64_type()});
  auto const callee_type = types()->NewFunctionType(void_type(), params_type);
  auto const callee =
      factory()->NewReference(callee_type, factory()->NewAtomicString(L"Foo"));
  auto const arguments = factory()->NewTupleInstruction(
      params_type, {factory()->NewInt32Literal(42),
                    true_value(),
                    factory()->NewInt64Literal(56),
                    factory()->NewInt16Literal(89),
                    factory()->NewFloat32Literal(1.2f),
                    factory()->NewFloat64Literal(3.4)});
  editor.Append(arguments);
  editor.Append(factory()->NewCallInstruction(callee, arguments));
  editor.Commit();
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  pcopy ECX, DL, R8, R9W, %arg[4], %arg[5] = 42, 1, 56l, 89, 1.2f, 3.4\n"
      "  call \"Foo\"\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Generate(function()));
}

TEST_F(GeneratorX64Test, Element) {
  auto const function = NewFunction(
      char_type(),
      types()->NewPointerType(types()->NewArrayType(char_type(), {-1})));
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const entry_instr = function->entry_block()->first_instruction();
  auto const element_instr = factory()->NewElementInstruction(
      entry_instr, factory()->NewInt32Literal(42));
  editor.Append(element_instr);
  auto const load_instr = factory()->NewLoadInstruction(element_instr);
  editor.Append(load_instr);
  editor.SetReturn(load_instr);
  editor.Commit();
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  mov %r1l = RCX\n"
      "  add %r2l = %r1l, 16\n"
      "  shl %r4 = 42, 1\n"
      "  add %r3l = %r2l, %r4\n"
      "  load %r5w = %r3l\n"
      "  zext EAX = %r5w\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
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
  ASSERT_EQ("", Validate(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  pcopy %r1, %r2l, %r3b, %f1d, %r4l = ECX, RDX, R8B, XMM3, %param[4]\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Generate(function));
}

TEST_F(GeneratorX64Test, ReturnInt32) {
  auto const function = NewFunction(int32_type(), void_type());
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.SetReturn(factory()->NewInt32Literal(42));
  editor.Commit();
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  lit EAX = 42\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Generate(function));
}

TEST_F(GeneratorX64Test, ReturnInt64) {
  auto const function = NewFunction(int64_type(), void_type());
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.SetReturn(factory()->NewInt64Literal(42));
  editor.Commit();
  ASSERT_EQ("", Validate(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  lit RAX = 42l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Generate(function));
}

}  // namespace
}  // namespace cg
}  // namespace elang
