// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/testing/lir_test_x64.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/lowering_x64_pass.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirLoweringX64Test
//
class LirLoweringX64Test : public testing::LirTest {
 protected:
  LirLoweringX64Test() = default;
  ~LirLoweringX64Test() = default;

  Function* CreateSampleFunction(Value type, int count);
  std::vector<Value> EmitCopyParameters(Editor* editor);

 private:
  DISALLOW_COPY_AND_ASSIGN(LirLoweringX64Test);
};

Function* LirLoweringX64Test::CreateSampleFunction(Value type, int count) {
  std::vector<Value> parameters;
  for (auto position = 0; position < count; ++position) {
    parameters.push_back(Target::ParameterAt(type, position));
  }
  return factory()->NewFunction(parameters);
}

std::vector<Value> LirLoweringX64Test::EmitCopyParameters(Editor* editor) {
  std::vector<Value> parameters;
  std::vector<Value> registers;
  for (auto const parameter : editor->function()->parameters()) {
    parameters.push_back(parameter);
    registers.push_back(factory()->NewRegister(parameter));
  }
  editor->Append(factory()->NewPCopyInstruction(registers, parameters));
  return registers;
}

// Test cases...

// int Foo(float64 x, float64 y) {
//   return x / y;
// }
#define DEFINE_FLOAT64_BINARY_OPERATION_TEST(Name, mnemonic)              \
  TEST_F(LirLoweringX64Test, Name##Float) {                               \
    auto const type = Value::Float64Type();                               \
    auto const function = CreateSampleFunction(type, 2);                  \
    auto const entry_block = function->entry_block();                     \
    Editor editor(factory(), function);                                   \
    editor.Edit(entry_block);                                             \
    auto const parameters = EmitCopyParameters(&editor);                  \
    auto output = NewRegister(type);                                      \
    editor.Append(                                                        \
        New##Name##Instruction(output, parameters[0], parameters[1]));    \
    editor.Append(NewCopyInstruction(Target::ReturnAt(type, 0), output)); \
    editor.SetReturn();                                                   \
    EXPECT_EQ("", Commit(&editor));                                       \
    ASSERT_EQ("", Validate(&editor));                                     \
                                                                          \
    RunPassForTesting<LoweringX64Pass>(&editor);                          \
    EXPECT_EQ(                                                            \
        "function1:\n"                                                    \
        "block1:\n"                                                       \
        "  // In: {}\n"                                                   \
        "  // Out: {block2}\n"                                            \
        "  entry XMM0D, XMM1D =\n"                                        \
        "  pcopy %f1d, %f2d = XMM0D, XMM1D\n"                             \
        "  mov %f4d = %f1d\n"                                             \
        "  " mnemonic                                                     \
        " %f5d = %f4d, %f2d\n"                                            \
        "  mov %f3d = %f5d\n"                                             \
        "  mov XMM0D = %f3d\n"                                            \
        "  ret block2\n"                                                  \
        "block2:\n"                                                       \
        "  // In: {block1}\n"                                             \
        "  // Out: {}\n"                                                  \
        "  exit\n",                                                       \
        FormatFunction(&editor));                                         \
  }

// int Foo(int x, int y) {
//   return x + y;
// }
#define DEFINE_INTEGER_BINARY_OPERATION_TEST(Name, mnemonic)              \
  TEST_F(LirLoweringX64Test, Name##Int) {                                 \
    auto const type = Value::Int32Type();                                 \
    auto const function = CreateSampleFunction(type, 2);                  \
    auto const entry_block = function->entry_block();                     \
    Editor editor(factory(), function);                                   \
    editor.Edit(entry_block);                                             \
    auto const parameters = EmitCopyParameters(&editor);                  \
    auto output = NewRegister(type);                                      \
    editor.Append(                                                        \
        New##Name##Instruction(output, parameters[0], parameters[1]));    \
    editor.Append(NewCopyInstruction(Target::ReturnAt(type, 0), output)); \
    editor.SetReturn();                                                   \
    EXPECT_EQ("", Commit(&editor));                                       \
    ASSERT_EQ("", Validate(&editor));                                     \
                                                                          \
    RunPassForTesting<LoweringX64Pass>(&editor);                          \
    EXPECT_EQ(                                                            \
        "function1:\n"                                                    \
        "block1:\n"                                                       \
        "  // In: {}\n"                                                   \
        "  // Out: {block2}\n"                                            \
        "  entry ECX, EDX =\n"                                            \
        "  pcopy %r1, %r2 = ECX, EDX\n"                                   \
        "  mov %r4 = %r1\n"                                               \
        "  " mnemonic                                                     \
        " %r5 = %r4, %r2\n"                                               \
        "  mov %r3 = %r5\n"                                               \
        "  mov EAX = %r3\n"                                               \
        "  ret block2\n"                                                  \
        "block2:\n"                                                       \
        "  // In: {block1}\n"                                             \
        "  // Out: {}\n"                                                  \
        "  exit\n",                                                       \
        FormatFunction(&editor));                                         \
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
DEFINE_FLOAT64_BINARY_OPERATION_TEST(Mul, "mul")

// int Foo(int x, int y) {
//   return x / y;
// }
TEST_F(LirLoweringX64Test, DivInt) {
  auto const type = Value::Int32Type();
  auto const function = CreateSampleFunction(type, 2);
  auto const entry_block = function->entry_block();
  Editor editor(factory(), function);
  editor.Edit(entry_block);
  auto const parameters = EmitCopyParameters(&editor);
  auto output = NewRegister(type);
  editor.Append(NewDivInstruction(output, parameters[0], parameters[1]));
  editor.Append(NewCopyInstruction(Target::ReturnAt(type, 0), output));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));

  RunPassForTesting<LoweringX64Pass>(&editor);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX, EDX =\n"
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

// int Foo(int x, int y) {
//   return x * y;
// }
TEST_F(LirLoweringX64Test, MulInt) {
  auto const type = Value::Int32Type();
  auto const function = CreateSampleFunction(type, 2);
  auto const entry_block = function->entry_block();
  Editor editor(factory(), function);
  editor.Edit(entry_block);
  auto const parameters = EmitCopyParameters(&editor);
  auto output = NewRegister(type);
  editor.Append(NewMulInstruction(output, parameters[0], parameters[1]));
  editor.Append(NewCopyInstruction(Target::ReturnAt(type, 0), output));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));
  ASSERT_EQ("", Validate(&editor));

  RunPassForTesting<LoweringX64Pass>(&editor);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX, EDX =\n"
      "  pcopy %r1, %r2 = ECX, EDX\n"
      "  mov EAX = %r1\n"
      "  x64.mul EAX, EDX = EAX, %r2\n"
      "  mov %r3 = EAX\n"
      "  mov EAX = %r3\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

// int Foo(int x, int y) {
//   var z = x << 5;
//   return x << y;
// }
#define DEFINE_SHIFT_OPERATION_TEST(Name, mnemonic)                            \
  TEST_F(LirLoweringX64Test, Name) {                                           \
    auto const type = Value::Int32Type();                                      \
    auto const function = CreateSampleFunction(type, 2);                       \
    auto const entry_block = function->entry_block();                          \
    Editor editor(factory(), function);                                        \
    editor.Edit(entry_block);                                                  \
    auto const parameters = EmitCopyParameters(&editor);                       \
    auto output = NewRegister(type);                                           \
    auto output2 = NewRegister(type);                                          \
    editor.Append(                                                             \
        New##Name##Instruction(output2, parameters[0], Value::SmallInt32(5))); \
    editor.Append(New##Name##Instruction(output, output2, parameters[1]));     \
    editor.Append(NewCopyInstruction(Target::ReturnAt(type, 0), output));      \
    editor.SetReturn();                                                        \
    EXPECT_EQ("", Commit(&editor));                                            \
    ASSERT_EQ("", Validate(&editor));                                          \
                                                                               \
    RunPassForTesting<LoweringX64Pass>(&editor);                               \
    EXPECT_EQ(                                                                 \
        "function1:\n"                                                         \
        "block1:\n"                                                            \
        "  // In: {}\n"                                                        \
        "  // Out: {block2}\n"                                                 \
        "  entry ECX, EDX =\n"                                                 \
        "  pcopy %r1, %r2 = ECX, EDX\n"                                        \
        "  mov %r5 = %r1\n"                                                    \
        "  " mnemonic                                                          \
        " %r6 = %r5, 5\n"                                                      \
        "  mov %r4 = %r6\n"                                                    \
        "  mov %r7 = %r4\n"                                                    \
        "  mov ECX = %r2\n"                                                    \
        "  " mnemonic                                                          \
        " %r8 = %r7, ECX\n"                                                    \
        "  mov %r3 = %r8\n"                                                    \
        "  mov EAX = %r3\n"                                                    \
        "  ret block2\n"                                                       \
        "block2:\n"                                                            \
        "  // In: {block1}\n"                                                  \
        "  // Out: {}\n"                                                       \
        "  exit\n",                                                            \
        FormatFunction(&editor));                                              \
  }

DEFINE_SHIFT_OPERATION_TEST(Shl, "shl")
DEFINE_SHIFT_OPERATION_TEST(Shr, "shr")

}  // namespace lir
}  // namespace elang
