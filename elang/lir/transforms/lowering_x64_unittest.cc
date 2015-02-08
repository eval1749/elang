// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/lowering_x64.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirLoweringX64Test
//
class LirLoweringX64Test : public testing::LirTest {
 protected:
  LirLoweringX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirLoweringX64Test);
};

// Test cases...

// int Foo(float64 x, float64 y) {
//   return x / y;
// }
#define DEFINE_FLOAT64_BINARY_OPERATION_TEST(Name, mnemonic)            \
  TEST_F(LirLoweringX64Test, Name##Float) {                             \
    auto const function = CreateFunctionEmptySample();                  \
    auto const entry_block = function->entry_block();                   \
    Editor editor(factory(), function);                                 \
    editor.Edit(entry_block);                                           \
    auto const type = Value(Value::Type::Float, ValueSize::Size64);     \
    auto const parameters = EmitCopyParameters(&editor, type, 2);       \
    auto output = NewRegister(type);                                    \
    editor.Append(                                                      \
        New##Name##Instruction(output, parameters[0], parameters[1]));  \
    editor.Append(NewCopyInstruction(Target::GetReturn(type), output)); \
    editor.SetReturn();                                                 \
    EXPECT_EQ("", Commit(&editor));                                     \
    ASSERT_EQ("", Validate(&editor));                                   \
                                                                        \
    X64LoweringPass(factory(), function).Run();                         \
    EXPECT_EQ(                                                          \
        "function1:\n"                                                  \
        "block1:\n"                                                     \
        "  // In: {}\n"                                                 \
        "  // Out: {block2}\n"                                          \
        "  entry\n"                                                     \
        "  pcopy %f1d, %f2d = XMM0, XMM1\n"                             \
        "  assign %f4d = %f1d\n"                                        \
        "  " mnemonic                                                   \
        " %f5d = %f4d, %f2d\n"                                          \
        "  mov %f3d = %f5d\n"                                           \
        "  mov XMM0 = %f3d\n"                                           \
        "  ret block2\n"                                                \
        "block2:\n"                                                     \
        "  // In: {block1}\n"                                           \
        "  // Out: {}\n"                                                \
        "  exit\n",                                                     \
        FormatFunction(&editor));                                       \
  }

// int Foo(int x, int y) {
//   return x + y;
// }
#define DEFINE_INTEGER_BINARY_OPERATION_TEST(Name, mnemonic)            \
  TEST_F(LirLoweringX64Test, Name##Int) {                               \
    auto const function = CreateFunctionEmptySample();                  \
    auto const entry_block = function->entry_block();                   \
    Editor editor(factory(), function);                                 \
    editor.Edit(entry_block);                                           \
    auto const type = Value(Value::Type::Integer, ValueSize::Size32);   \
    auto const parameters = EmitCopyParameters(&editor, type, 2);       \
    auto output = NewRegister(type);                                    \
    editor.Append(                                                      \
        New##Name##Instruction(output, parameters[0], parameters[1]));  \
    editor.Append(NewCopyInstruction(Target::GetReturn(type), output)); \
    editor.SetReturn();                                                 \
    EXPECT_EQ("", Commit(&editor));                                     \
    ASSERT_EQ("", Validate(&editor));                                   \
                                                                        \
    X64LoweringPass(factory(), function).Run();                         \
    EXPECT_EQ(                                                          \
        "function1:\n"                                                  \
        "block1:\n"                                                     \
        "  // In: {}\n"                                                 \
        "  // Out: {block2}\n"                                          \
        "  entry\n"                                                     \
        "  pcopy %r1, %r2 = ECX, EDX\n"                                 \
        "  assign %r4 = %r1\n"                                          \
        "  " mnemonic                                                   \
        " %r5 = %r4, %r2\n"                                             \
        "  mov %r3 = %r5\n"                                             \
        "  mov EAX = %r3\n"                                             \
        "  ret block2\n"                                                \
        "block2:\n"                                                     \
        "  // In: {block1}\n"                                           \
        "  // Out: {}\n"                                                \
        "  exit\n",                                                     \
        FormatFunction(&editor));                                       \
  }

#define DEFINE_BINARY_OPERATION_TEST(Name, mnemonic)   \
  DEFINE_FLOAT64_BINARY_OPERATION_TEST(Name, mnemonic) \
  DEFINE_INTEGER_BINARY_OPERATION_TEST(Name, mnemonic)

DEFINE_BINARY_OPERATION_TEST(Add, "add")
DEFINE_BINARY_OPERATION_TEST(BitAnd, "and")
DEFINE_BINARY_OPERATION_TEST(BitOr, "or")
DEFINE_BINARY_OPERATION_TEST(BitXor, "xor")
DEFINE_BINARY_OPERATION_TEST(Sub, "sub")

DEFINE_FLOAT64_BINARY_OPERATION_TEST(Div, "div")

// int Foo(int x, int y) {
//   return x / y;
// }
TEST_F(LirLoweringX64Test, DivInt) {
  auto const function = CreateFunctionEmptySample();
  auto const entry_block = function->entry_block();
  Editor editor(factory(), function);
  editor.Edit(entry_block);
  auto const type = Value(Value::Type::Integer, ValueSize::Size32);
  auto const parameters = EmitCopyParameters(&editor, type, 2);
  auto output = NewRegister(type);
  editor.Append(NewDivInstruction(output, parameters[0], parameters[1]));
  editor.Append(NewCopyInstruction(Target::GetReturn(type), output));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));

  X64LoweringPass(factory(), function).Run();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      "  pcopy %r1, %r2 = ECX, EDX\n"
      "  mov EAX = %r1\n"
      "  xor EDX = EDX, EDX\n"
      "  x64.div EAX, EDX = EAX, EDX, %r2\n"
      "  mov %r3 = EAX\n"  // redundant copy instructions.
      "  mov EAX = %r3\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

}  // namespace lir
}  // namespace elang
