// Copyright 2014-2015 Project Vogue. All rights reserved.
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
#include "elang/lir/testing/lir_test.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirLiteralsTest offers HIR factories.
//
class LirLiteralsTest : public testing::LirTest {
 protected:
  LirLiteralsTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirLiteralsTest);
};

TEST_F(LirLiteralsTest, Function) {
  auto const function = CreateFunctionEmptySample();
  EXPECT_TRUE(
      function->entry_block()->first_instruction()->is<EntryInstruction>());
  EXPECT_TRUE(
      function->exit_block()->first_instruction()->is<ExitInstruction>());
}

TEST_F(LirLiteralsTest, SimpleLiterals) {
  std::stringstream stream;
  stream << *GetLiteral(NewFloat32Value(3.2f)) << std::endl;
  stream << *GetLiteral(NewFloat64Value(6.4f)) << std::endl;
  stream << *GetLiteral(NewInt32Value(1 << 30)) << std::endl;
  stream << *GetLiteral(NewInt64Value(static_cast<int64_t>(1) << 40))
         << std::endl;

  EXPECT_EQ(
      "3.2f\n"
      "6.4\n"
      "1073741824\n"
      "1099511627776l\n",
      stream.str());
}

TEST_F(LirLiteralsTest, StringLiteral) {
  base::string16 sample(L"xy\a\b\f\n\r\t\uABCD\v\\z");
  sample[1] = 0;
  std::stringstream stream;
  stream << *GetLiteral(NewStringValue(sample));
  EXPECT_EQ("\"x\\0\\a\\b\\f\\n\\r\\t\\uABCD\\v\\\\z\"", stream.str());
}

}  // namespace lir
}  // namespace elang
