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
// LirParallelCopyExpanderTest
//
class LirParallelCopyExpanderTest : public testing::LirTest {
 protected:
  LirParallelCopyExpanderTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirParallelCopyExpanderTest);
};

// Test cases...

TEST_F(LirParallelCopyExpanderTest, Basic) {
  auto const function = CreateFunctionEmptySample();
  Editor editor(factory(), function);
}

}  // namespace lir
}  // namespace elang
