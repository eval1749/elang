// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/factory.h"
#include "elang/lir/editor.h"
#include "elang/lir/transforms/prepare_phi_inversion_pass.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirPreparePhiInversionTest
//
class LirPreparePhiInversionTest : public testing::LirTest {
 protected:
  LirPreparePhiInversionTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirPreparePhiInversionTest);
};

// Test cases...

TEST_F(LirPreparePhiInversionTest, Basic) {
  auto const function = CreateFunctionWithCriticalEdge2();
  Editor editor(factory(), function);
  Run<PreparePhiInversionPass>(&editor);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3}\n"
      "  entry void =\n"
      "  mov %r1 = ECX\n"
      "  jmp block3\n"
      "block3:\n"
      "  // In: {block1, block4}\n"
      "  // Out: {block4, block5}\n"
      "  br %b2, block5, block4\n"
      "block4:\n"
      "  // In: {block3}\n"
      "  // Out: {block3, block7}\n"
      "  br %b3, block7, block3\n"
      "block7:\n"
      "  // In: {block4}\n"
      "  // Out: {block6}\n"
      "  jmp block6\n"
      "block5:\n"
      "  // In: {block3}\n"
      "  // Out: {block6}\n"
      "  use %r1\n"
      "  jmp block6\n"
      "block6:\n"
      "  // In: {block5, block7}\n"
      "  // Out: {block2}\n"
      "  phi %r2 = block7 42, block5 %r1\n"
      "  mov EAX = %r2\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block6}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

}  // namespace lir
}  // namespace elang
