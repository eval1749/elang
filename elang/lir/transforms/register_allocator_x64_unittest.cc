// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/editor.h"
#include "elang/lir/transforms/lowering_x64.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirRegisterAllocatorX64Test
//
class LirRegisterAllocatorX64Test : public testing::LirTest {
 protected:
  LirRegisterAllocatorX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirRegisterAllocatorX64Test);
};

// Test cases...

//  function1:
//    block1:
//      // In: {}
//      // Out: {block2}
//      entry
//      pcopy %r1l, %r2l = RCX, RDX
//      assign %r4l = %r1l
//      add %r5l = %r4l, %r2l
//      mov %r3l = %r5l
//      mov RAX = %r3l
//      ret block2
//    block2:
//      // In: {block1}
//      // Out: {}
//      exit
TEST_F(LirRegisterAllocatorX64Test, SampleAdd) {
  auto const function = CreateFunctionSampleAdd();
  {
    Editor editor(factory(), function);
    Run<X64LoweringPass>(&editor);
  }
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block2}\n"
      "  entry\n"
      // TODO(eval1749) we should allocate |R1|, |R2| instead of |R10|, |R11|.
      "* mov r10l = r1l\n"
      "* mov r11l = r2l\n"
      "  pcopy r10l, r11l = r1l, r2l\n"
      "  assign r10l = r10l\n"
      "  add r10l = r10l, r11l\n"
      "  mov r10l = r10l\n"
      "  mov r0l = r10l\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block1}\n"
      "  // Out: {}\n"
      "  exit\n",
      Allocate(function));
}

//  function1:
//  block1:
//    // In: {}
//    // Out: {block3}
//    entry
//    jmp block3
//  block3:
//    // In: {block1, block4}
//    // Out: {block5}
//    jmp block5
//  block4:
//    // In: {}
//    // Out: {block3, block6}
//    br %b2, block6, block3
//  block6:
//    // In: {block4}
//    // Out: {block5}
//    jmp block5
//  block5:
//    // In: {block3, block6}
//    // Out: {block2}
//    phi %r1 = block6 42, block3 39
//    mov EAX = %r1
//    ret block2
//  block2:
//    // In: {block5}
//    // Out: {}
//    exit
TEST_F(LirRegisterAllocatorX64Test, WithCriticalEdge) {
  auto const function = CreateFunctionWithCriticalEdge();
  {
    Editor editor(factory(), function);
    Run<X64LoweringPass>(&editor);
  }

  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  // In: {}\n"
      "  // Out: {block3}\n"
      "  entry\n"
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
      "* lit r10 = #42\n"  // from phi-instruction
      "  jmp block6\n"
      "block5:\n"
      "  // In: {block3}\n"
      "  // Out: {block6}\n"
      "* lit r10 = #39\n"  // from phi-instruction
      "  jmp block6\n"
      "block6:\n"
      "  // In: {block5, block7}\n"
      "  // Out: {block2}\n"
      "  phi r10 = block7 #42, block5 #39\n"
      "  mov r0 = r10\n"
      "  ret block2\n"
      "block2:\n"
      "  // In: {block6}\n"
      "  // Out: {}\n"
      "  exit\n",
      Allocate(function));
}

}  // namespace lir
}  // namespace elang
