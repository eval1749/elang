// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/factory.h"
#include "elang/lir/editor.h"
#include "elang/lir/transforms/remove_critical_edges_pass.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirRemoveCriticalEdgesTest
//
class LirRemoveCriticalEdgesTest : public testing::LirTest {
 protected:
  LirRemoveCriticalEdgesTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirRemoveCriticalEdgesTest);
};

// Test cases...

// Before transform:
//    function1:
//    block1:
//      // In: {}
//      // Out: {block3}
//      entry void =
//      mov %r1 = ECX
//      jmp block3
//    block3:
//      // In: {block1, block4}
//      // Out: {block4, block5}
//      br %b2, block5, block4
//    block4:
//      // In: {block3}
//      // Out: {block3, block6}
//      br %b3, block6, block3
//    block5:
//      // In: {block3}
//      // Out: {block6}
//      jmp block6
//    block6:
//      // In: {block4, block5}
//      // Out: {block2}
//      phi %r2 = block4 42, block5 %r1
//      mov EAX = %r2
//      ret block2
//    block2:
//      // In: {block6}
//      // Out: {}
//      exit
//
TEST_F(LirRemoveCriticalEdgesTest, Basic) {
  auto const function = CreateFunctionWithCriticalEdge2();
  Editor editor(factory(), function);
  Run<RemoveCriticalEdgesPass>(&editor);
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3}\n"
      "  entry ECX =\n"
      "  mov %r1 = ECX\n"
      "  jmp block3\n"
      "block3:\n"
      "  // In: {block1, block7}\n"
      "  // Out: {block4, block5}\n"
      "  br %b2, block5, block4\n"
      "block4:\n"
      "  // In: {block3}\n"
      "  // Out: {block7, block8}\n"
      "  br %b3, block8, block7\n"
      "block8:\n"
      "  // In: {block4}\n"
      "  // Out: {block6}\n"
      "  jmp block6\n"
      "block7:\n"
      "  // In: {block4}\n"
      "  // Out: {block3}\n"
      "  jmp block3\n"
      "block5:\n"
      "  // In: {block3}\n"
      "  // Out: {block6}\n"
      "  jmp block6\n"
      "block6:\n"
      "  // In: {block5, block8}\n"
      "  // Out: {block2}\n"
      "  phi %r2 = block8 42, block5 %r1\n"
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
