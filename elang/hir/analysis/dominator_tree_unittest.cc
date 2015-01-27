// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/base/zone.h"
#include "elang/hir/analysis/dominator_tree_builder.h"
#include "elang/hir/analysis/graph.h"
#include "elang/hir/values.h"
#include "elang/hir/testing/hir_test.h"

namespace elang {
namespace hir {
namespace {

typedef DominatorTree::Node Node;

class HirDominatorTreehTest : public testing::HirTest {
 protected:
  HirDominatorTreehTest() = default;
  ~HirDominatorTreehTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(HirDominatorTreehTest);
};

std::unordered_set<Node*> NodeSet(const std::vector<Node*>& list) {
  return std::unordered_set<Node*>(list.begin(), list.end());
}

std::unordered_set<Node*> NodeSet(const ZoneVector<Node*>& list) {
  return std::unordered_set<Node*>(list.begin(), list.end());
}

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

TEST_F(HirDominatorTreehTest, SampleFunction) {
  auto const function = NewSampleFunction();
  ControlFlowGraph cfg(function);
  Zone zone;
  auto const dom = DominatorTreeBuilder(&zone, &cfg).Build();

  std::vector<Node*> nodes;
  size_t counter = 0;
  for (auto block : function->basic_blocks()) {
    if (counter)
      nodes.push_back(dom->node_of(block));
    ++counter;
  }
  // remove exit block.
  nodes.pop_back();

  auto const entry_node = dom->node_of(function->entry_block());
  auto const exit_node = dom->node_of(function->exit_block());

  // Immediate dominator
  EXPECT_EQ(nullptr, entry_node->parent()) << entry_node;
  EXPECT_EQ(nodes[5], exit_node->parent()) << exit_node;
  EXPECT_EQ(entry_node, nodes[0]->parent()) << nodes[0];
  EXPECT_EQ(nodes[0], nodes[1]->parent()) << nodes[1];
  EXPECT_EQ(nodes[1], nodes[2]->parent()) << nodes[2];
  EXPECT_EQ(nodes[2], nodes[3]->parent()) << nodes[3];
  EXPECT_EQ(nodes[1], nodes[4]->parent()) << nodes[4];
  EXPECT_EQ(nodes[0], nodes[5]->parent()) << nodes[5];
  EXPECT_EQ(nodes[2], nodes[6]->parent()) << nodes[6];

  // Dominance children
  EXPECT_EQ(NodeSet({nodes[1], nodes[5]}), NodeSet(nodes[0]->children()))
      << nodes[0];
  EXPECT_EQ(NodeSet({nodes[2], nodes[4]}), NodeSet(nodes[1]->children()))
      << nodes[1];
  EXPECT_EQ(NodeSet({nodes[3], nodes[6]}), NodeSet(nodes[2]->children()))
      << nodes[2];
  EXPECT_EQ(NodeSet({}), NodeSet(nodes[3]->children())) << nodes[3];
  EXPECT_EQ(NodeSet({}), NodeSet(nodes[4]->children())) << nodes[4];
  EXPECT_EQ(NodeSet({exit_node}), NodeSet(nodes[5]->children())) << nodes[5];
  EXPECT_EQ(NodeSet({}), NodeSet(nodes[6]->children())) << nodes[6];

// Dominance frontiers
#if 0
  EXPECT_EQ(NodeSet({exit_node}), NodeSet(nodes[0]->frontiers()))
      << nodes[0];
#else
  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B0)=[exit].
  EXPECT_EQ(NodeSet({}), NodeSet(nodes[0]->frontiers())) << nodes[0];
#endif
  EXPECT_EQ(NodeSet({nodes[1], nodes[5]}), NodeSet(nodes[1]->frontiers()))
      << nodes[1];
  EXPECT_EQ(NodeSet({nodes[2], nodes[4]}), NodeSet(nodes[2]->frontiers()))
      << nodes[2];
  EXPECT_EQ(NodeSet({nodes[2], nodes[4]}), NodeSet(nodes[3]->frontiers()))
      << nodes[3];
  EXPECT_EQ(NodeSet({nodes[1], nodes[5]}), NodeSet(nodes[4]->frontiers()))
      << nodes[4];
#if 0
  EXPECT_EQ(NodeSet({exit_node}), NodeSet(nodes[5]->frontiers()))
      << nodes[5];
#else
  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B5)=[exit].
  EXPECT_EQ(NodeSet({}), NodeSet(nodes[5]->frontiers())) << nodes[5];
#endif
  EXPECT_EQ(NodeSet({nodes[3]}), NodeSet(nodes[6]->frontiers())) << nodes[6];
}

}  // namespace
}  // namespace hir
}  // namespace elang
