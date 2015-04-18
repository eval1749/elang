// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_set>

#include "elang/base/analysis/dominator_tree_builder.h"
#include "elang/base/graphs/flow_graph.h"
#include "elang/base/graphs/graph_editor.h"
#include "elang/base/graphs/graph_test_support.h"

namespace elang {
namespace {

using testing::Block;
using testing::Function;
typedef DominatorTree<Function> MyDominatorTree;

struct PrintableNodes {
  std::vector<MyDominatorTree::Node*> nodes;

  explicit PrintableNodes(const MyDominatorTree::Nodes& node_list)
      : nodes(node_list.begin(), node_list.end()) {
    std::sort(nodes.begin(), nodes.end(),
              [](MyDominatorTree::Node* a, MyDominatorTree::Node* b) {
      return a->value()->id() < b->value()->id();
    });
  }
};

PrintableNodes Printable(const MyDominatorTree::Nodes& nodes) {
  return PrintableNodes(nodes);
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableNodes& printable) {
  ostream << "[";
  auto separator = "";
  for (auto const node : printable.nodes) {
    ostream << separator << node->value()->id();
    separator = ", ";
  }
  return ostream << "]";
}

std::ostream& operator<<(std::ostream& ostream,
                         const MyDominatorTree::Node& node) {
  ostream << "{parent: ";
  if (node.parent())
    ostream << node.parent()->value()->id();
  else
    ostream << "none";
  ostream << ", children: " << Printable(node.children())
          << ", frontiers: " << Printable(node.frontiers()) << "}";
  return ostream;
}

//////////////////////////////////////////////////////////////////////
//
// DominatorTreeTest
//
class DominatorTreeTest : public testing::GraphTestBase {
 protected:
  DominatorTreeTest() = default;
  ~DominatorTreeTest() = default;

  static std::string ToString(const MyDominatorTree::Node* node) {
    std::stringstream ostream;
    ostream << *node;
    return ostream.str();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(DominatorTreeTest);
};

// Test cases...

// entry and exit blocks only.
TEST_F(DominatorTreeTest, Basic) {
  Function function;
  auto const entry_block = NewBlock(-1);
  auto const exit_block = NewBlock(-2);

  Function::Editor editor(&function);
  editor.AppendNode(entry_block);
  editor.AppendNode(exit_block);
  editor.AddEdge(entry_block, exit_block);

  auto const dominator_tree =
      DominatorTreeBuilder<Function, ForwardFlowGraph<Function>>(&function)
          .Build();
  auto const entry_node = dominator_tree->TreeNodeOf(entry_block);
  auto const exit_node = dominator_tree->TreeNodeOf(exit_block);

  EXPECT_EQ("{parent: none, children: [-2], frontiers: []}",
            ToString(entry_node));
  EXPECT_EQ("{parent: -1, children: [], frontiers: []}", ToString(exit_node));
}

TEST_F(DominatorTreeTest, BasicReverse) {
  Function function;
  auto const entry_block = NewBlock(-1);
  auto const exit_block = NewBlock(-2);

  Function::Editor editor(&function);
  editor.AppendNode(entry_block);
  editor.AppendNode(exit_block);
  editor.AddEdge(entry_block, exit_block);

  auto const dominator_tree =
      DominatorTreeBuilder<Function, BackwardFlowGraph<Function>>(&function)
          .Build();
  auto const entry_node = dominator_tree->TreeNodeOf(entry_block);
  auto const exit_node = dominator_tree->TreeNodeOf(exit_block);

  EXPECT_EQ("{parent: -2, children: [], frontiers: []}", ToString(entry_node));
  EXPECT_EQ("{parent: none, children: [-1], frontiers: []}",
            ToString(exit_node));
}

TEST_F(DominatorTreeTest, SampleFunction) {
  MakeSampleGraph1();

  auto const dom = DominatorTreeBuilder<Function, ForwardFlowGraph<Function>>(
                       function()).Build();

  auto const entry_block = function()->first_node();
  auto const exit_block = function()->last_node();

  std::vector<MyDominatorTree::Node*> nodes;
  for (auto block : function()->nodes()) {
    if (block == entry_block || block == exit_block)
      continue;
    nodes.push_back(dom->TreeNodeOf(block));
  }

  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B0)=[exit].
  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B5)=[exit].
  EXPECT_EQ("{parent: none, children: [0], frontiers: []}",
            ToString(dom->TreeNodeOf(entry_block)));
  EXPECT_EQ("{parent: 6, children: [], frontiers: []}",
            ToString(dom->TreeNodeOf(exit_block)));

  EXPECT_EQ("{parent: -1, children: [1, 6], frontiers: []}",
            ToString(nodes[0]));
  EXPECT_EQ("{parent: 0, children: [2, 4], frontiers: [1, 6]}",
            ToString(nodes[1]));
  EXPECT_EQ("{parent: 1, children: [3, 5], frontiers: [2, 4]}",
            ToString(nodes[2]));
  EXPECT_EQ("{parent: 2, children: [], frontiers: [2, 4]}", ToString(nodes[3]));
  EXPECT_EQ("{parent: 1, children: [], frontiers: [1, 6]}", ToString(nodes[4]));
  EXPECT_EQ("{parent: 2, children: [], frontiers: [3]}", ToString(nodes[5]));
  EXPECT_EQ("{parent: 0, children: [-2], frontiers: []}", ToString(nodes[6]));
}

TEST_F(DominatorTreeTest, SampleFunctionReverse) {
  MakeSampleGraph1();

  auto const dom = DominatorTreeBuilder<Function, BackwardFlowGraph<Function>>(
                       function()).Build();

  auto const entry_block = function()->first_node();
  auto const exit_block = function()->last_node();

  std::vector<MyDominatorTree::Node*> nodes;
  for (auto block : function()->nodes()) {
    if (block == entry_block || block == exit_block)
      continue;
    nodes.push_back(dom->TreeNodeOf(block));
  }

  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B0)=[exit].
  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B5)=[exit].
  EXPECT_EQ("{parent: 0, children: [], frontiers: []}",
            ToString(dom->TreeNodeOf(entry_block)));
  EXPECT_EQ("{parent: none, children: [6], frontiers: []}",
            ToString(dom->TreeNodeOf(exit_block)));

  EXPECT_EQ("{parent: 6, children: [-1], frontiers: []}", ToString(nodes[0]));
  EXPECT_EQ("{parent: 4, children: [], frontiers: [0, 4]}", ToString(nodes[1]));
  EXPECT_EQ("{parent: 3, children: [], frontiers: [1, 3]}", ToString(nodes[2]));
  EXPECT_EQ("{parent: 4, children: [2, 5], frontiers: [1, 3]}",
            ToString(nodes[3]));
  EXPECT_EQ("{parent: 6, children: [1, 3], frontiers: [0, 4]}",
            ToString(nodes[4]));
  EXPECT_EQ("{parent: 3, children: [], frontiers: [2]}", ToString(nodes[5]));
  EXPECT_EQ("{parent: -2, children: [0, 4], frontiers: []}",
            ToString(nodes[6]));
}

}  // namespace
}  // namespace elang
