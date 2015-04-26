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

}  // namespace translator
}  // namespace elang
