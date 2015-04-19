// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <unordered_set>

#include "elang/base/analysis/loop_tree_builder.h"
#include "elang/base/graphs/flow_graph.h"
#include "elang/base/graphs/graph_editor.h"
#include "elang/base/graphs/graph_test_support.h"

namespace elang {
namespace testing {

using MyLoopTree = LoopTree<Function>;
using MyLoopTreeNode = MyLoopTree::TreeNode;

struct PrintableLoopTreeNodes {
  std::vector<MyLoopTreeNode*> nodes;
};

PrintableLoopTreeNodes Printable(const ZoneVector<MyLoopTreeNode*> nodes) {
  PrintableLoopTreeNodes printable;
  printable.nodes.insert(printable.nodes.end(), nodes.begin(), nodes.end());
  std::sort(printable.nodes.begin(), printable.nodes.end(),
            [](MyLoopTreeNode* a, MyLoopTreeNode* b) {
              return a->entry()->id() < b->entry()->id();
            });
  return printable;
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableLoopTreeNodes& printable) {
  ostream << "{";
  auto separator = "";
  for (auto const node : printable.nodes) {
    ostream << separator << node->entry();
    separator = ", ";
  }
  return ostream << "}";
}

std::ostream& operator<<(std::ostream& ostream, MyLoopTreeNode::Kind kind) {
  static const char* const kinds[] = {"multiple", "root", "single"};
  auto const it = std::begin(kinds) + static_cast<size_t>(kind);
  return ostream << (it < std::end(kinds) ? *it : "INVALID");
}

std::ostream& operator<<(std::ostream& ostream, const MyLoopTreeNode* node) {
  if (!node)
    return ostream << "nil";
  return ostream << node->entry();
}

std::ostream& operator<<(std::ostream& ostream, const MyLoopTreeNode& node) {
  ostream << "{";
  ostream << "kind: " << node.kind() << ", ";
  ostream << "parent: " << node.parent() << ", ";
  ostream << "depth: " << node.depth() << ", ";
  ostream << "entry: " << node.entry() << ", ";
  ostream << "children: " << Printable(node.children()) << ", ";
  ostream << "nodes: " << Printable(node.nodes()) << "}";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const MyLoopTree& tree) {
  for (auto const node : tree)
    ostream << *node << std::endl;
  return ostream;
}

std::string ToString(const MyLoopTree& tree) {
  std::stringstream ostream;
  ostream << tree;
  return ostream.str();
}

}  // namespace testing

namespace {

using testing::Block;
using testing::Function;
using testing::Printable;
using testing::ToString;

//////////////////////////////////////////////////////////////////////
//
// LoopTreeTest
//
class LoopTreeTest : public testing::GraphTestBase {
 protected:
  LoopTreeTest() = default;
  ~LoopTreeTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LoopTreeTest);
};

// Test cases...

// entry and exit blocks only.
TEST_F(LoopTreeTest, Basic) {
  Function::Editor editor(function());

  auto const entry_block = NewBlock(-1);
  auto const exit_block = NewBlock(-2);

  editor.AppendNode(entry_block);
  editor.AppendNode(exit_block);
  editor.AddEdge(entry_block, exit_block);

  auto const loop_tree = LoopTreeBuilder<Function>(function()).Build();
  EXPECT_EQ(
      "{kind: root, parent: nil, depth: 0, entry: ENTRY, children: {}, "
      "nodes: {EXIT}}\n",
      ToString(*loop_tree));
  EXPECT_EQ(loop_tree->NodeOf(entry_block), loop_tree->NodeOf(exit_block));
}

TEST_F(LoopTreeTest, CaseC) {
  auto const entry_block = NewBlock(-1);
  auto const exit_block = NewBlock(-2);

  Function::Editor editor(function());
  editor.AppendNode(entry_block);
  editor.AppendNode(exit_block);

  std::array<Block*, 6> blocks;
  auto id = 0;
  for (auto& ref : blocks) {
    ref = NewBlock(id);
    editor.AppendNode(ref);
    ++id;
  }

  //    ENTRY DFSP=1
  //      |
  //      B0 DFSP=2
  //      |
  //  +---B1 DFSP=3
  //  V   |
  //  B3  B2 DFSP=4
  //  |   |
  //  B4<-+ DFSP=5
  //  |
  //  B5 DFSP=6
  //  |
  //  EXIT
  //  Pre-Order: ENTRY:1, B0:2, B1:3, B2:4, B4:5, B5:6, B3:4
  editor.AddEdge(entry_block, blocks[0]);
  editor.AddEdge(blocks[0], blocks[1]);
  editor.AddEdge(blocks[1], blocks[2]);
  editor.AddEdge(blocks[1], blocks[3]);
  editor.AddEdge(blocks[2], blocks[4]);
  editor.AddEdge(blocks[3], blocks[4]);
  editor.AddEdge(blocks[4], blocks[5]);

  auto const loop_tree = LoopTreeBuilder<Function>(function()).Build();
  EXPECT_EQ(
      "{kind: root, parent: nil, depth: 0, entry: ENTRY, children: {}, "
      "nodes: {B0, B1, B2, B3, B4, B5}}\n",
      ToString(*loop_tree));
}

TEST_F(LoopTreeTest, CaseD) {
  auto const entry_block = NewBlock(-1);
  auto const exit_block = NewBlock(-2);

  Function::Editor editor(function());
  editor.AppendNode(entry_block);
  editor.AppendNode(exit_block);

  std::array<Block*, 7> blocks;
  auto id = 0;
  for (auto& ref : blocks) {
    ref = NewBlock(id);
    editor.AppendNode(ref);
    ++id;
  }

  //    ENTRY
  //      |
  //      B0 (DFSP=2) position in DFS spanning tree.
  //      |
  //      B1<-------+ (DFSP=3)
  //      |         |
  //      B2----+   | (DFSP=4)
  //      |     |   |
  //      B3    B4  | (DFSP=5) (DFSP=4)
  //      |     |   |
  //      +---> B5  | (DFSP=6)
  //            |   |
  //            B6--+ (DFSP=7)
  //            |
  //            EXIT
  //  Pre-Order: ENTRY:1, B0:2, B1:3, B2:4, B3:5, B5:6, B6:7, B4:4
  editor.AddEdge(entry_block, blocks[0]);
  editor.AddEdge(blocks[0], blocks[1]);
  editor.AddEdge(blocks[1], blocks[2]);
  editor.AddEdge(blocks[2], blocks[3]);
  editor.AddEdge(blocks[3], blocks[5]);
  editor.AddEdge(blocks[4], blocks[5]);
  editor.AddEdge(blocks[5], blocks[6]);
  editor.AddEdge(blocks[1], blocks[4]);
  editor.AddEdge(blocks[3], blocks[5]);
  editor.AddEdge(blocks[6], blocks[1]);

  auto const loop_tree = LoopTreeBuilder<Function>(function()).Build();
  EXPECT_EQ(
      "{kind: root, parent: nil, depth: 0, entry: ENTRY, children: {B1}, "
      "nodes: {B0}}\n"
      "{kind: single, parent: ENTRY, depth: 1, entry: B1, children: {}, nodes: "
      "{B2, B3, B4, B5, B6}}\n",
      ToString(*loop_tree));
}

TEST_F(LoopTreeTest, CaseE) {
  auto const entry_block = NewBlock(-1);
  auto const exit_block = NewBlock(-2);

  Function::Editor editor(function());
  editor.AppendNode(entry_block);
  editor.AppendNode(exit_block);

  std::array<Block*, 8> blocks;
  auto id = 0;
  for (auto& ref : blocks) {
    ref = NewBlock(id);
    editor.AppendNode(ref);
    ++id;
  }

  //    ENTRY
  //      |
  //      B0<-----------+
  //      |             |
  //      B1----+       |
  //      |     |       |
  //      B2    B4      |
  //      |     |       |
  //      B3    B5<-+   |
  //      |     |   |   |
  //      +---> B6  |   |
  //            |   |   |
  //            B7--+---+
  //            |
  //            EXIT
  // Pre-Order: ENTRY:1, B0:2, B1:3, B2:4, B3:5, B6:6, B7:7, EXIT:8, B5:8, B4:4
  editor.AddEdge(entry_block, blocks[0]);
  editor.AddEdge(blocks[0], blocks[1]);
  editor.AddEdge(blocks[1], blocks[2]);
  editor.AddEdge(blocks[2], blocks[3]);
  editor.AddEdge(blocks[3], blocks[6]);

  editor.AddEdge(blocks[1], blocks[4]);

  editor.AddEdge(blocks[4], blocks[5]);
  editor.AddEdge(blocks[5], blocks[6]);
  editor.AddEdge(blocks[6], blocks[7]);
  editor.AddEdge(blocks[7], exit_block);

  editor.AddEdge(blocks[7], blocks[5]);
  editor.AddEdge(blocks[7], blocks[0]);

  auto const loop_tree = LoopTreeBuilder<Function>(function()).Build();
  EXPECT_EQ(
      "{kind: root, parent: nil, depth: 0, entry: ENTRY, children: {B0}, "
      "nodes: {EXIT}}\n"
      "{kind: single, parent: ENTRY, depth: 1, entry: B0, children: {B6}, "
      "nodes: {B1, B2, B3, B4}}\n"
      "{kind: multiple, parent: B0, depth: 2, entry: B6, children: {B5}, "
      "nodes: {B7}}\n"
      "{kind: multiple, parent: B6, depth: 3, entry: B5, children: {}, nodes: "
      "{}}\n",
      ToString(*loop_tree));
}

TEST_F(LoopTreeTest, SampleFunction) {
  MakeSampleGraph1();
  // Pre-Order: ENTRY:1, B0:2, B1:3, B2:4, B3:5, B4:6, B6:7, EXIT:8
  auto const loop_tree = LoopTreeBuilder<Function>(function()).Build();
  EXPECT_EQ(
      "{kind: root, parent: nil, depth: 0, entry: ENTRY, children: {B1}, "
      "nodes: {EXIT, B0, B6}}\n"
      "{kind: single, parent: ENTRY, depth: 1, entry: B1, children: {B2}, "
      "nodes: {B4}}\n"
      "{kind: single, parent: B1, depth: 2, entry: B2, children: {}, nodes: "
      "{B3, B5}}\n",
      ToString(*loop_tree));
}

TEST_F(LoopTreeTest, WhileContinues) {
  auto const entry_block = NewBlock(-1);
  auto const exit_block = NewBlock(-2);

  Function::Editor editor(function());
  editor.AppendNode(entry_block);
  editor.AppendNode(exit_block);

  std::array<Block*, 6> blocks;
  auto id = 0;
  for (auto& ref : blocks) {
    ref = NewBlock(id);
    editor.AppendNode(ref);
    ++id;
  }

  //    ENTRY
  //      |
  //      B0
  //      |
  //      B1<---+
  //      |     |
  //      B2--->+
  //      |     |
  //      B3--->+
  //      |     |
  //      B4--->+
  //      |
  //      B5
  //      |
  //      EXIT
  // Pre-Order: ENTRY:1, B0:2, B1:3, B2:4, B3:5, B4:6, B5:7, EXIT:8
  editor.AddEdge(entry_block, blocks[0]);
  editor.AddEdge(blocks[0], blocks[1]);
  editor.AddEdge(blocks[1], blocks[2]);
  editor.AddEdge(blocks[2], blocks[3]);
  editor.AddEdge(blocks[3], blocks[4]);
  editor.AddEdge(blocks[4], blocks[5]);
  editor.AddEdge(blocks[5], exit_block);

  editor.AddEdge(blocks[2], blocks[1]);
  editor.AddEdge(blocks[3], blocks[1]);
  editor.AddEdge(blocks[4], blocks[1]);

  auto const loop_tree = LoopTreeBuilder<Function>(function()).Build();
  EXPECT_EQ(
      "{kind: root, parent: nil, depth: 0, entry: ENTRY, children: {B1}, "
      "nodes: {EXIT, B0, B5}}\n"
      "{kind: single, parent: ENTRY, depth: 1, entry: B1, children: {}, nodes: "
      "{B2, B3, B4}}\n",
      ToString(*loop_tree));
}
}  // namespace
}  // namespace elang
