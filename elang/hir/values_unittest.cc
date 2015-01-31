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
  EXPECT_EQ(
      "function1 void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      Format());
  EXPECT_FALSE(entry_block()->HasPredecessor());
  EXPECT_TRUE(exit_block()->HasPredecessor());
}

TEST_F(HirValuesTest, Function2) {
  EXPECT_EQ(
      "function2 void(bool)\n"
      "block3:\n"
      "  // In:\n"
      "  // Out: block5\n"
      "  bool %b5 = entry\n"
      "  br block5\n"
      "block5:\n"
      "  // In: block3\n"
      "  // Out: block6 block10\n"
      "  br %b5, block6, block10\n"
      "block6:\n"
      "  // In: block5 block9\n"
      "  // Out: block7 block9\n"
      "  br %b5, block7, block9\n"
      "block7:\n"
      "  // In: block6 block8\n"
      "  // Out: block8 block11\n"
      "  br %b5, block8, block11\n"
      "block8:\n"
      "  // In: block7 block11\n"
      "  // Out: block7 block9\n"
      "  br %b5, block7, block9\n"
      "block9:\n"
      "  // In: block6 block8\n"
      "  // Out: block6 block10\n"
      "  br %b5, block6, block10\n"
      "block10:\n"
      "  // In: block5 block9\n"
      "  // Out: block4\n"
      "  ret void, block4\n"
      "block11:\n"
      "  // In: block7\n"
      "  // Out: block8\n"
      "  br block8\n"
      "block4:\n"
      "  // In: block10\n"
      "  // Out:\n"
      "  exit\n",
      Format(NewSampleFunction()));
}

// Literals
TEST_F(HirValuesTest, Literal) {
  std::stringstream ostream;
  EXPECT_EQ(false_value(), factory()->NewBoolLiteral(false));
  EXPECT_EQ(true_value(), factory()->NewBoolLiteral(true));

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
