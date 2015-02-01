// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/cg/testing/cg_test.h"

#include "elang/cg/generator.h"

namespace elang {
namespace cg {
namespace {

//////////////////////////////////////////////////////////////////////
//
// GeneratorTest
//
class GeneratorTest : public testing::CgTest {
 protected:
  GeneratorTest() = default;
  ~GeneratorTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(GeneratorTest);
};

// Test cases...

TEST_F(GeneratorTest, Basic) {
  Generator generator(lir_factory(), function());
  auto const lir_function = generator.Generate();
  EXPECT_EQ(
      "Function\n"
      "block1:\n"
      "  entry\n"
      "  ret\n"
      "\n"
      "block2:\n"
      "  exit\n",
      Format(lir_function));
}

}  // namespace
}  // namespace cg
}  // namespace elang
