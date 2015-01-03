// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <memory>
#include <sstream>

#include "base/strings/string16.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LiteralsTest offers HIR factories.
//
class LiteralsTest : public ::testing::Test {
 protected:
  LiteralsTest();

  Factory* factory() { return factory_.get(); }

 private:
  std::unique_ptr<Factory> factory_;
};

LiteralsTest::LiteralsTest() : factory_(new Factory()) {
}

//////////////////////////////////////////////////////////////////////
//
// Function
//
#ifdef ELANG_TARGET_ARCH_X64
TEST_F(LiteralsTest, Function) {
  auto const function = factory()->NewFunction();
  Editor editor(factory(), function);
  auto const entry_block = function->entry_block();
  {
    Editor::ScopedEdit scope(&editor);
    editor.Edit(entry_block);
    auto const call = factory()->NewCallInstruction();
    editor.SetInput(call, 0, factory()->NewStringValue(L"Foo"));
    editor.InsertBefore(call, entry_block->last_instruction());
  }
  EXPECT_TRUE(entry_block->first_instruction()->is<EntryInstruction>());
  EXPECT_TRUE(
      function->exit_block()->first_instruction()->is<ExitInstruction>());

  std::stringstream stream;
  TextFormatter formatter(factory(), &stream);
  formatter.FormatFunction(function);
  EXPECT_EQ(
      "Function\n"
      "block1:\n"
      "  entry\n"
      "  call \"Foo\"\n"
      "  ret\n"
      "\n"
      "block2:\n"
      "  exit\n",
      stream.str());
}
#else
#error "We must define LiteralsTest.Function test."
#endif

//////////////////////////////////////////////////////////////////////
//
// Simple Literals
//
TEST_F(LiteralsTest, SimpleLiterals) {
  std::stringstream stream;
  stream << *factory()->GetLiteral(factory()->NewFloat32Value(3.2f))
         << std::endl;
  stream << *factory()->GetLiteral(factory()->NewFloat64Value(6.4f))
         << std::endl;
  stream << *factory()->GetLiteral(factory()->NewInt32Value(1 << 30))
         << std::endl;
  stream << *factory()->GetLiteral(factory()->NewInt64Value(
                static_cast<int64_t>(1) << 40)) << std::endl;

  EXPECT_EQ(
      "3.2f\n"
      "6.4\n"
      "1073741824\n"
      "1099511627776l\n",
      stream.str());
}

//////////////////////////////////////////////////////////////////////
//
// String Literals
//
TEST_F(LiteralsTest, StringLiteral) {
  base::string16 sample(L"xy\a\b\f\n\r\t\uABCD\v\\z");
  sample[1] = 0;
  std::stringstream stream;
  stream << *factory()->GetLiteral(factory()->NewStringValue(sample));
  EXPECT_EQ("\"x\\0\\a\\b\\f\\n\\r\\t\\uABCD\\v\\\\z\"", stream.str());
}

}  // namespace lir
}  // namespace elang
