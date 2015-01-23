// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
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
  EXPECT_TRUE(entry_block()->first_instruction()->is<EntryInstruction>());
  EXPECT_TRUE(exit_block()->first_instruction()->is<ExitInstruction>());

  editor()->Edit(entry_block());
  editor()->SetInput(entry_block()->last_instruction(), 0,
                     factory()->NewStringLiteral(L"foo"));
  EXPECT_TRUE(editor()->Commit());
  EXPECT_EQ(
      "function1 void(void)\n"
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
      Format());
}

// Literals
TEST_F(HirValuesTest, Literal) {
  std::stringstream ostream;
  ostream << *factory()->NewBoolLiteral(false) << std::endl;
  ostream << *factory()->NewBoolLiteral(true) << std::endl;

  ostream << *factory()->NewCharLiteral('a') << std::endl;
  ostream << *factory()->NewCharLiteral('\n') << std::endl;
  ostream << *factory()->NewCharLiteral('\'') << std::endl;
  ostream << *factory()->NewCharLiteral('\\') << std::endl;
  ostream << *factory()->NewCharLiteral(0x1234) << std::endl;

  ostream << *factory()->NewFloat32Literal(12.34f) << std::endl;
  ostream << *factory()->NewFloat64Literal(12.34) << std::endl;

  ostream << *factory()->NewInt16Literal(-1234) << std::endl;
  ostream << *factory()->NewInt32Literal(-1234) << std::endl;
  ostream << *factory()->NewInt64Literal(-1234) << std::endl;
  ostream << *factory()->NewInt8Literal(-123) << std::endl;

  ostream << *factory()->NewUInt16Literal(1234) << std::endl;
  ostream << *factory()->NewUInt32Literal(1234) << std::endl;
  ostream << *factory()->NewUInt64Literal(1234) << std::endl;
  ostream << *factory()->NewUInt8Literal(123) << std::endl;

  EXPECT_EQ(
      "false\n"
      "true\n"
      "'a'\n"
      "'\\n'\n"
      "'\\\''\n"
      "'\\\\'\n"
      "'\\u1234'\n"
      "12.34f\n"
      "12.34\n"
      "int16(-1234)\n"
      "-1234\n"
      "-1234l\n"
      "int8(-123)\n"
      "uint16(1234)\n"
      "1234u\n"
      "1234ul\n"
      "uint8(123)\n",
      ostream.str());
}

}  // namespace hir
}  // namespace elang
