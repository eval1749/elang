// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/factory.h"
#include "elang/lir/editor.h"
#include "elang/lir/transforms/prepare_phi_inversion.h"

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
  auto const function = CreateFunctionWithCriticalEdge();
  Editor editor(factory(), function);
  PreparePhiInversionPass pass(&editor);
  pass.Run();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3}\n"
      "  entry\n"
      "  jmp block3\n"
      "block3:\n"
      "  // In: {block1, block4}\n"
      "  // Out: {block5}\n"
      "  jmp block5\n"
      "block4:\n"
      "  // In: {}\n"
      "  // Out: {block3, block6}\n"
      "  br %b2, block6, block3\n"
      "block6:\n"  // new block |block6| is inserted.
      "  // In: {block4}\n"
      "  // Out: {block5}\n"
      "  jmp block5\n"
      "block5:\n"
      "  // In: {block3, block6}\n"
      "  // Out: {block2}\n"
      "  phi %r1 = block6 42, block3 39\n"
      "  mov EAX = %r1\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block5}\n"
      "  // Out: {}\n"
      "  exit\n",
      FormatFunction(&editor));
}

}  // namespace lir
}  // namespace elang
