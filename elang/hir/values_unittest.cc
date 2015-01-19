// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <sstream>

#include "elang/base/zone.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/formatters/text_formatter.h"
#include "elang/hir/instructions.h"
#include "elang/hir/values.h"
#include "elang/hir/types.h"
#include "elang/hir/testing/hir_test.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// HirValuesTest offers HIR factories.
//
class HirValuesTest : public testing::HirTest {
 protected:
  HirValuesTest() = default;
  ~HirValuesTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(HirValuesTest);
};

// Functions
TEST_F(HirValuesTest, Function) {
  auto const void_type = types()->GetVoidType();
  auto const function_type = types()->NewFunctionType(void_type, void_type);
  auto const function = factory()->NewFunction(function_type);
  auto const entry_block = function->entry_block();
  EXPECT_TRUE(entry_block->first_instruction()->is<EntryInstruction>());
  EXPECT_TRUE(
      function->exit_block()->first_instruction()->is<ExitInstruction>());

  Editor editor(factory(), function);
  editor.Edit(entry_block);
  editor.SetInput(entry_block->last_instruction(), 0,
                  factory()->NewStringLiteral(L"foo"));
  editor.Commit();

  std::stringstream stream;
  TextFormatter formatter(&stream);
  formatter.FormatFunction(function);
  EXPECT_EQ(
      "Function void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  ret \"foo\", block2\n"
      "\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      stream.str());
}

}  // namespace hir
}  // namespace elang
