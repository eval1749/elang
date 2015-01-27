// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/zone.h"
#include "elang/hir/analysis/dominator_tree_builder.h"
#include "elang/hir/analysis/graph.h"
#include "elang/hir/values.h"
#include "elang/hir/testing/hir_test.h"

namespace elang {
namespace hir {
namespace {

class HirDominatorTreehTest : public testing::HirTest {
 protected:
  HirDominatorTreehTest() = default;
  ~HirDominatorTreehTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(HirDominatorTreehTest);
};

TEST_F(HirDominatorTreehTest, Basic) {
  ControlFlowGraph cfg(function());
  Zone zone;

  auto dominator_tree = DominatorTreeBuilder(&zone, &cfg).Build();

  auto const entry_node = dominator_tree->node_of(entry_block());
  EXPECT_EQ(nullptr, entry_node->parent());
  ASSERT_EQ(1u, entry_node->children().size());
  EXPECT_EQ(exit_block(), entry_node->children().front()->value());
  ASSERT_EQ(0u, entry_node->frontiers().size());

  auto const exit_node = dominator_tree->node_of(exit_block());
  EXPECT_EQ(entry_node, exit_node->parent());
  EXPECT_EQ(0u, exit_node->children().size());
  ASSERT_EQ(0u, exit_node->frontiers().size());
}

}  // namespace
}  // namespace hir
}  // namespace elang
