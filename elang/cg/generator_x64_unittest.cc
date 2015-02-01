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
// GeneratorX64Test
//
class GeneratorX64Test : public testing::CgTest {
 protected:
  GeneratorX64Test() = default;
  ~GeneratorX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(GeneratorX64Test);
};

// Test cases...

TEST_F(GeneratorX64Test, Basic) {
  Generator generator(lir_factory(), function());
  auto const lir_function = generator.Generate();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Format(lir_function));
}

}  // namespace
}  // namespace cg
}  // namespace elang
