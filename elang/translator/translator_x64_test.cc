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

}  // namespace translator
}  // namespace elang
