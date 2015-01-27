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

  std::vector<int> rpo_blocks;
  for (auto value : cfg.ReversePostOrderList())
    rpo_blocks.push_back(value->as<BasicBlock>()->id());
  EXPECT_EQ((std::vector<int>{1, 2}), rpo_blocks);

  EXPECT_EQ(entry_block(), cfg.entry());

  auto successors = cfg.SuccessorsOf(entry_block());
  ASSERT_EQ(1u, successors.size());
  EXPECT_EQ(exit_block(), *successors.begin());

  auto predecessors = cfg.PredecessorsOf(exit_block());
  ASSERT_EQ(1u, predecessors.size());
  EXPECT_EQ(entry_block(), *predecessors.begin());
}

TEST_F(HirGraphTest, Sample) {
  // See HirValuesTest.SampleFunction2 for instructions.
  auto const function = NewSampleFunction();
  ControlFlowGraph cfg(function);

  std::vector<int> rpo_blocks;
  for (auto value : cfg.ReversePostOrderList())
    rpo_blocks.push_back(value->as<BasicBlock>()->id());
  EXPECT_EQ((std::vector<int>{3, 5, 6, 7, 11, 8, 9, 10, 4}), rpo_blocks);
  EXPECT_EQ(function->entry_block()->id(), rpo_blocks.front());
  EXPECT_EQ(function->exit_block()->id(), rpo_blocks.back());

  std::vector<int> po_blocks;
  for (auto value : cfg.PostOrderList())
    po_blocks.push_back(value->as<BasicBlock>()->id());
  EXPECT_EQ((std::vector<int>{4, 10, 9, 8, 11, 7, 6, 5, 3}), po_blocks);
  EXPECT_EQ(function->entry_block()->id(), po_blocks.back());
  EXPECT_EQ(function->exit_block()->id(), po_blocks.front());

  std::vector<int> rpre_blocks;
  for (auto value : cfg.ReversePreOrderList())
    rpre_blocks.push_back(value->as<BasicBlock>()->id());
  EXPECT_EQ((std::vector<int>{11, 4, 10, 9, 8, 7, 6, 5, 3}), rpre_blocks);

  std::vector<int> pre_blocks;
  for (auto value : cfg.PreOrderList())
    pre_blocks.push_back(value->as<BasicBlock>()->id());
  EXPECT_EQ((std::vector<int>{3, 5, 6, 7, 8, 9, 10, 4, 11}), pre_blocks);
}

}  // namespace
}  // namespace hir
}  // namespace elang
