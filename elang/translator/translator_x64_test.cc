// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/translator/testing/translator_test.h"

#include "elang/optimizer/editor.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace translator {

//////////////////////////////////////////////////////////////////////
//
// TranslatorX64Test
//
class TranslatorX64Test : public testing::TranslatorTest {
 protected:
  TranslatorX64Test() = default;
  ~TranslatorX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TranslatorX64Test);
};

#define DEFINE_RET_TEST(Name, name, line)                          \
  TEST_F(TranslatorX64Test, Ret##Name) {                           \
    auto const function = NewFunction(name##_type(), void_type()); \
    ir::Editor editor(factory(), function);                        \
    auto const entry_node = function->entry_node();                \
    auto const effect = NewGetEffect(entry_node);                  \
                                                                   \
    editor.Edit(entry_node);                                       \
    editor.SetRet(effect, New##Name(42));                          \
    ASSERT_EQ("", Commit(&editor));                                \
                                                                   \
    EXPECT_EQ(                                                     \
        "function1:\n"                                             \
        "block1:\n"                                                \
        "  // In: {}\n"                                            \
        "  // Out: {block2}\n"                                     \
        "  entry\n"                                                \
        "  " line                                                  \
        "\n"                                                       \
        "  ret block2\n"                                           \
        "block2:\n"                                                \
        "  // In: {block1}\n"                                      \
        "  // Out: {}\n"                                           \
        "  exit\n",                                                \
        Translate(editor));                                        \
  }

DEFINE_RET_TEST(Float32, float32, "lit XMM0 = 42f")
DEFINE_RET_TEST(Float64, float64, "lit XMM0 = 42")
DEFINE_RET_TEST(Int16, int16, "lit EAX = 42")
DEFINE_RET_TEST(Int32, int32, "lit EAX = 42")
DEFINE_RET_TEST(Int64, int64, "lit RAX = 42l")
DEFINE_RET_TEST(UInt16, uint16, "lit EAX = 42")
DEFINE_RET_TEST(UInt32, uint32, "lit EAX = 42")
DEFINE_RET_TEST(UInt64, uint64, "lit RAX = 42l")

TEST_F(TranslatorX64Test, ElementNode) {
  auto const function =
      NewFunction(NewPointerType(int32_type()),
                  NewPointerType(NewArrayType(int32_type(), {-1})));
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const array = NewParameter(entry_node, 0);
  editor.SetRet(effect, NewElement(array, NewInt32(42)));
  ASSERT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry RCX =\n"
      "  pcopy %r1l = RCX\n"
      "  add %r2l = %r1l, 16l\n"
      "  shl %r3 = 42, 2\n"
      "  sext %r4l = %r3\n"
      "  add %r5l = %r2l, %r4l\n"
      "  mov RAX = %r5l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, EntryNode) {
  auto const function = NewFunction(void_type(), void_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect, void_value());
  ASSERT_EQ("", Commit(&editor));
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
      Translate(editor));
}

TEST_F(TranslatorX64Test, EntryNode1) {
  auto const function = NewFunction(int32_type(), int32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect, NewParameter(entry_node, 0));
  ASSERT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  mov EAX = %r1\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, EntryNode2) {
  auto const function = NewFunction(
      float32_type(), NewTupleType({float32_type(), float32_type()}));
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const param0 = NewParameter(entry_node, 0);
  auto const param1 = NewParameter(entry_node, 1);
  editor.SetRet(effect, NewFloatAdd(param0, param1));
  ASSERT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry XMM0, XMM1 =\n"
      "  pcopy %f1, %f2 = XMM0, XMM1\n"
      "  add %f3 = %f1, %f2\n"
      "  mov XMM0 = %f3\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, IfNode) {
  auto const function = NewFunction(int32_type(), int32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const param0 = NewParameter(entry_node, 0);
  auto const condition =
      NewIntCmp(ir::IntCondition::SignedLessThan, param0, NewInt32(42));
  auto const if_node = editor.SetBranch(condition);
  ASSERT_EQ("", Commit(&editor));

  editor.Edit(NewIfTrue(if_node));
  editor.SetRet(effect, NewInt32(12));
  ASSERT_EQ("", Commit(&editor));

  editor.Edit(NewIfFalse(if_node));
  editor.SetRet(effect, NewInt32(34));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3, block4}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  cmp_lt %b2 = %r1, 42\n"
      "  br %b2, block3, block4\n"
      "block3:\n"
      "  // In: {block1}\n"
      "  // Out: {block2}\n"
      "  lit EAX = 12\n"
      "  ret block2\n"
      "block4:\n"
      "  // In: {block1}\n"
      "  // Out: {block2}\n"
      "  lit EAX = 34\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block3, block4}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, IntCmpNode) {
  auto const function = NewFunction(bool_type(), int32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const param0 = NewParameter(entry_node, 0);
  editor.SetRet(effect, NewIntCmp(ir::IntCondition::SignedLessThan, param0,
                                  NewInt32(42)));
  ASSERT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  cmp_lt %b2 = %r1, 42\n"
      // TODO(eval1749) We should use "if" instruction to convert |bool| value
      // to |int32| value.
      "  lit EAX = %b2\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, LengthNode) {
  auto const function = NewFunction(
      int32_type(), NewPointerType(NewArrayType(int32_type(), {-1})));
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const array = NewParameter(entry_node, 0);
  editor.SetRet(effect, NewLength(array, 0));
  ASSERT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry RCX =\n"
      "  pcopy %r1l = RCX\n"
      "  load %r3 = %r1l, %r1l, 8\n"
      "  mov EAX = %r3\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, LoadNode) {
  auto const function = NewFunction(char_type(), NewPointerType(char_type()));
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const ptr = NewParameter(entry_node, 0);
  editor.SetRet(effect, NewLoad(effect, ptr, ptr));
  ASSERT_EQ("", Commit(&editor));
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry RCX =\n"
      "  pcopy %r1l = RCX\n"
      "  load %r2w = %r1l, %r1l, 0\n"
      "  zext EAX = %r2w\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, PhiNode) {
  auto const function = NewFunction(
      int32_type(), NewTupleType({bool_type(), int32_type(), int32_type()}));
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  auto const if_node = editor.SetBranch(NewParameter(entry_node, 0));
  ASSERT_EQ("", Commit(&editor));

  auto const ret_control = NewMerge({});

  editor.Edit(NewIfTrue(if_node));
  editor.SetJump(ret_control);
  ASSERT_EQ("", Commit(&editor));

  editor.Edit(NewIfFalse(if_node));
  editor.SetJump(ret_control);
  ASSERT_EQ("", Commit(&editor));

  editor.Edit(ret_control);
  auto const phi = NewPhi(int32_type(), ret_control);
  editor.SetPhiInput(phi, ret_control->control(0), NewParameter(entry_node, 1));
  editor.SetPhiInput(phi, ret_control->control(1), NewParameter(entry_node, 2));
  editor.SetRet(effect, phi);
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3, block5}\n"
      "  entry CL, EDX, R8D =\n"
      "  pcopy %r1b, %r2, %r3 = CL, EDX, R8D\n"
      "  cmp_ne %b2 = %r1b, 0\n"
      "  br %b2, block3, block5\n"
      "block3:\n"
      "  // In: {block1}\n"
      "  // Out: {block4}\n"
      "  jmp block4\n"
      "block4:\n"
      "  // In: {block3, block5}\n"
      "  // Out: {block2}\n"
      "  phi %r4 = block3 %r2, block5 %r3\n"
      "  mov EAX = %r4\n"
      "  ret block2\n"
      "block5:\n"
      "  // In: {block1}\n"
      "  // Out: {block4}\n"
      "  jmp block4\n"
      "block2:\n"
      "  // In: {block4}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeFloat32ToFloat64) {
  auto const function = NewFunction(float64_type(), float32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(float64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry XMM0 =\n"
      "  pcopy %f1 = XMM0\n"
      "  ext %f2d = %f1\n"
      "  mov XMM0 = %f2d\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeFloat32ToInt64) {
  auto const function = NewFunction(int64_type(), float32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(int64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry XMM0 =\n"
      "  pcopy %f1 = XMM0\n"
      "  sconv %r1l = %f1\n"
      "  mov RAX = %r1l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeFloat32ToUInt64) {
  auto const function = NewFunction(uint64_type(), float32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(uint64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry XMM0 =\n"
      "  pcopy %f1 = XMM0\n"
      "  uconv %r1l = %f1\n"
      "  mov RAX = %r1l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeFloat64oFloat32) {
  auto const function = NewFunction(float32_type(), float64_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(float32_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry XMM0 =\n"
      "  pcopy %f1d = XMM0\n"
      "  trunc %f2 = %f1d\n"
      "  mov XMM0 = %f2\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeInt32ToFloat64) {
  auto const function = NewFunction(float64_type(), int32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(float64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  sconv %f1d = %r1\n"
      "  mov XMM0 = %f1d\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeInt32ToInt64) {
  auto const function = NewFunction(int64_type(), int32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(int64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  sext %r2l = %r1\n"
      "  mov RAX = %r2l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeInt32ToUInt64) {
  auto const function = NewFunction(uint64_type(), int32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(uint64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  sext %r2l = %r1\n"
      "  mov RAX = %r2l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeInt64ToInt32) {
  auto const function = NewFunction(int32_type(), int64_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(int32_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry RCX =\n"
      "  pcopy %r1l = RCX\n"
      "  trunc %r2 = %r1l\n"
      "  mov EAX = %r2\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeInt64ToUInt32) {
  auto const function = NewFunction(uint32_type(), int64_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(uint32_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry RCX =\n"
      "  pcopy %r1l = RCX\n"
      "  trunc %r2 = %r1l\n"
      "  mov EAX = %r2\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodePtrToInt64) {
  auto const function =
      NewFunction(uint64_type(), NewPointerType(int32_type()));
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(uint64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry RCX =\n"
      "  pcopy %r1l = RCX\n"
      "  mov RAX = %r1l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeUInt32ToFloat64) {
  auto const function = NewFunction(float64_type(), uint32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(float64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  uconv %f1d = %r1\n"
      "  mov XMM0 = %f1d\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeUInt32ToInt64) {
  auto const function = NewFunction(int64_type(), uint32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(int64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  zext %r2l = %r1\n"
      "  mov RAX = %r2l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

TEST_F(TranslatorX64Test, StaticCastNodeUInt32ToUInt64) {
  auto const function = NewFunction(uint64_type(), uint32_type());
  ir::Editor editor(factory(), function);
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);

  editor.Edit(entry_node);
  editor.SetRet(effect,
                NewStaticCast(uint64_type(), NewParameter(entry_node, 0)));
  ASSERT_EQ("", Commit(&editor));

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry ECX =\n"
      "  pcopy %r1 = ECX\n"
      "  zext %r2l = %r1\n"
      "  mov RAX = %r2l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Translate(editor));
}

}  // namespace translator
}  // namespace elang
