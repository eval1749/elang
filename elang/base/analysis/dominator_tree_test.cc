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
                         const MyDominatorTree::Node* node) {
  if (!node)
    return ostream << "nil";
  return ostream << node->value();
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableNodes& printable) {
  ostream << "[";
  auto separator = "";
  for (auto const node : printable.nodes) {
    ostream << separator << node;
    separator = ", ";
  }
  return ostream << "]";
}

std::ostream& operator<<(std::ostream& ostream,
                         const MyDominatorTree::Node& node) {
  ostream << "{parent: " << node.parent() << ","
          << " children: " << Printable(node.children()) << ","
          << " frontiers: " << Printable(node.frontiers()) << "}";
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
    std::ostringstream ostream;
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

  EXPECT_EQ("{parent: nil, children: [EXIT], frontiers: []}",
            ToString(entry_node));
  EXPECT_EQ("{parent: ENTRY, children: [], frontiers: []}",
            ToString(exit_node));
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

  EXPECT_EQ("{parent: EXIT, children: [], frontiers: []}",
            ToString(entry_node));
  EXPECT_EQ("{parent: nil, children: [ENTRY], frontiers: []}",
            ToString(exit_node));
}

TEST_F(DominatorTreeTest, SampleFunction) {
  MakeSampleGraph1();

  auto const dom = DominatorTreeBuilder<Function, ForwardFlowGraph<Function>>(
                       function()).Build();

  auto const entry_block = function()->first_node();
  auto const exit_block = function()->last_node();

  std::vector<Block*> blocks;
  std::vector<MyDominatorTree::Node*> nodes;
  for (auto block : function()->nodes()) {
    if (block == entry_block || block == exit_block)
      continue;
    blocks.push_back(block);
    nodes.push_back(dom->TreeNodeOf(block));
  }

  // Dominator tree of sample graph1:
  //  ENTRY
  //    |
  //    B0
  //    / \
  //   B1  B6
  //   / \  \
  //  B2 B4  EXIT
  //  / \
  //  B3 B5

  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B0)=[exit].
  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B5)=[exit].
  EXPECT_EQ("{parent: nil, children: [B0], frontiers: []}",
            ToString(dom->TreeNodeOf(entry_block)));
  EXPECT_EQ("{parent: B6, children: [], frontiers: []}",
            ToString(dom->TreeNodeOf(exit_block)));

  EXPECT_EQ("{parent: ENTRY, children: [B1, B6], frontiers: []}",
            ToString(nodes[0]));
  EXPECT_EQ("{parent: B0, children: [B2, B4], frontiers: [B1, B6]}",
            ToString(nodes[1]));
  EXPECT_EQ("{parent: B1, children: [B3, B5], frontiers: [B2, B4]}",
            ToString(nodes[2]));
  EXPECT_EQ("{parent: B2, children: [], frontiers: [B2, B4]}",
            ToString(nodes[3]));
  EXPECT_EQ("{parent: B1, children: [], frontiers: [B1, B6]}",
            ToString(nodes[4]));
  EXPECT_EQ("{parent: B2, children: [], frontiers: [B3]}", ToString(nodes[5]));
  EXPECT_EQ("{parent: B0, children: [EXIT], frontiers: []}",
            ToString(nodes[6]));

  EXPECT_EQ(blocks[2], dom->CommonAncestorOf(blocks[3], blocks[5]));
  EXPECT_EQ(blocks[1], dom->CommonAncestorOf(blocks[3], blocks[4]));
  EXPECT_EQ(blocks[0], dom->CommonAncestorOf(blocks[3], blocks[6]));
}

TEST_F(DominatorTreeTest, SampleFunctionReverse) {
  MakeSampleGraph1();

  auto const dom = DominatorTreeBuilder<Function, BackwardFlowGraph<Function>>(
                       function()).Build();

  auto const entry_block = function()->first_node();
  auto const exit_block = function()->last_node();

  std::vector<Block*> blocks;
  std::vector<MyDominatorTree::Node*> nodes;
  for (auto block : function()->nodes()) {
    if (block == entry_block || block == exit_block)
      continue;
    blocks.push_back(block);
    nodes.push_back(dom->TreeNodeOf(block));
  }

  // Post dominator tree of sample graph1:
  //    EXIT
  //     |
  //     B6
  //    /  \
  //   B0   B4
  //   |     /\
  //  ENTRY B1 B3
  //           |
  //           B5

  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B0)=[exit].
  // TODO(eval1749) Until we have pseudo edge from entry to exit, we don't
  // have DF(B5)=[exit].
  EXPECT_EQ("{parent: B0, children: [], frontiers: []}",
            ToString(dom->TreeNodeOf(entry_block)));
  EXPECT_EQ("{parent: nil, children: [B6], frontiers: []}",
            ToString(dom->TreeNodeOf(exit_block)));

  EXPECT_EQ("{parent: B6, children: [ENTRY], frontiers: []}",
            ToString(nodes[0]));
  EXPECT_EQ("{parent: B4, children: [], frontiers: [B0, B4]}",
            ToString(nodes[1]));
  EXPECT_EQ("{parent: B3, children: [], frontiers: [B1, B3]}",
            ToString(nodes[2]));
  EXPECT_EQ("{parent: B4, children: [B2, B5], frontiers: [B1, B3]}",
            ToString(nodes[3]));
  EXPECT_EQ("{parent: B6, children: [B1, B3], frontiers: [B0, B4]}",
            ToString(nodes[4]));
  EXPECT_EQ("{parent: B3, children: [], frontiers: [B2]}", ToString(nodes[5]));
  EXPECT_EQ("{parent: EXIT, children: [B0, B4], frontiers: []}",
            ToString(nodes[6]));

  EXPECT_EQ(blocks[6], dom->CommonAncestorOf(blocks[3], blocks[6]));
  EXPECT_EQ(blocks[4], dom->CommonAncestorOf(blocks[3], blocks[1]));
  EXPECT_EQ(blocks[6], dom->CommonAncestorOf(blocks[3], entry_block));
}

}  // namespace
}  // namespace elang
