// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/analysis/graph.h"
#include "elang/hir/values.h"
#include "elang/hir/testing/hir_test.h"

namespace elang {
namespace hir {
namespace {

class HirGraphTest : public testing::HirTest {
 protected:
  HirGraphTest() = default;
  ~HirGraphTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(HirGraphTest);
};

TEST_F(HirGraphTest, Basic) {
  ControlFlowGraph cfg(function());

  EXPECT_EQ(entry_block(), cfg.entry());

  auto successors = cfg.successors_of(entry_block());
  ASSERT_EQ(1u, successors.size());
  EXPECT_EQ(exit_block(), *successors.begin());

  auto predecessors = cfg.predecessors_of(exit_block());
  ASSERT_EQ(1u, predecessors.size());
  EXPECT_EQ(entry_block(), *predecessors.begin());
}

}  // namespace
}  // namespace hir
}  // namespace elang
