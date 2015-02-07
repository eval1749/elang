// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "elang/base/graphs/graph_editor.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

class Block;

class Function : public Graph<Function, Block> {
 public:
  Function() = default;
  ~Function() = default;
};

class Block : public Graph<Function, Block>::Node, public ZoneAllocated {
 public:
  Block(Zone* zone, int id) : Node(zone), id_(id) {}
  ~Block() = default;

  int id() const { return id_; }

 private:
  int id_;
};

struct PrintableBlocks {
  std::vector<Block*> blocks;

  explicit PrintableBlocks(const ZoneUnorderedSet<Block*>& block_set)
      : blocks(block_set.begin(), block_set.end()) {
    std::sort(blocks.begin(), blocks.end(),
              [](Block* a, Block* b) { return a->id() < b->id(); });
  }
};

std::ostream& operator<<(std::ostream& ostream, const PrintableBlocks& blocks) {
  ostream << "{";
  auto separator = "";
  for (auto const block : blocks.blocks) {
    ostream << separator << block->id();
    separator = ", ";
  }
  ostream << "}";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Block& block) {
  ostream << "{id:" << block.id()
          << " predecessors:" << PrintableBlocks(block.predecessors())
          << " successors:" << PrintableBlocks(block.successors()) << "}";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Function& function) {
  for (auto const block : function.nodes())
    ostream << *block << std::endl;
  return ostream;
}

std::string ToString(Function* function) {
  std::stringstream ostream;
  ostream << *function;
  return ostream.str();
}

std::string ToString(const OrderedList<Block*>& list) {
  std::stringstream ostream;
  ostream << "[";
  auto separator = "";
  for (auto const block : list) {
    ostream << separator << block->id();
    separator = ", ";
  }
  ostream << "]";
  return ostream.str();
}

//////////////////////////////////////////////////////////////////////
//
// GraphTest
//
class GraphTest : public ::testing::Test, public ZoneOwner {
 protected:
  GraphTest() = default;

  Block* block_at(int index) { return blocks_[index]; }
  Function* function() { return &function_; }

  Block* NewBlock(int id);

  // ::testing::Test
  void SetUp() override;

 private:
  Function function_;
  std::vector<Block*> blocks_;

  DISALLOW_COPY_AND_ASSIGN(GraphTest);
};

Block* GraphTest::NewBlock(int id) {
  auto const block = new (zone()) Block(zone(), id);
  blocks_.push_back(block);
  return block;
}

// Build graph
//      1
//     / \
//     2  3
//     \  /
//      4
void GraphTest::SetUp() {
  Function::Editor editor(function());
  auto const block1 = NewBlock(1);
  auto const block2 = NewBlock(2);
  auto const block3 = NewBlock(3);
  auto const block4 = NewBlock(4);

  editor.AppendNode(block1);
  editor.AppendNode(block2);
  editor.AppendNode(block3);
  editor.AppendNode(block4);
  editor.AddEdge(block1, block2);
  editor.AddEdge(block1, block3);
  editor.AddEdge(block2, block4);
  editor.AddEdge(block3, block4);
}

// Test cases...

TEST_F(GraphTest, AddEdge) {
  std::stringstream ostream;
  ostream << *function();
  EXPECT_EQ(
      "{id:1 predecessors:{} successors:{2, 3}}\n"
      "{id:2 predecessors:{1} successors:{4}}\n"
      "{id:3 predecessors:{1} successors:{4}}\n"
      "{id:4 predecessors:{2, 3} successors:{}}\n",
      ToString(function()));
  auto const block1 = block_at(0);
  auto const block2 = block_at(1);
  auto const block3 = block_at(2);
  auto const block4 = block_at(3);

  EXPECT_TRUE(function()->HasEdge(block1, block2));
  EXPECT_TRUE(function()->HasEdge(block1, block3));
  EXPECT_TRUE(function()->HasEdge(block2, block4));
  EXPECT_TRUE(function()->HasEdge(block3, block4));

  EXPECT_FALSE(function()->HasEdge(block2, block1));
  EXPECT_FALSE(function()->HasEdge(block3, block1));

  EXPECT_FALSE(block1->HasPredecessor());
  EXPECT_TRUE(block1->HasSuccessor());
  EXPECT_FALSE(block1->HasMoreThanOnePredecessors());

  EXPECT_TRUE(block2->HasPredecessor());
  EXPECT_TRUE(block2->HasSuccessor());
  EXPECT_FALSE(block2->HasMoreThanOnePredecessors());

  EXPECT_TRUE(block3->HasPredecessor());
  EXPECT_TRUE(block3->HasSuccessor());
  EXPECT_FALSE(block3->HasMoreThanOnePredecessors());

  EXPECT_TRUE(block4->HasPredecessor());
  EXPECT_FALSE(block4->HasSuccessor());
  EXPECT_TRUE(block4->HasMoreThanOnePredecessors());
}

TEST_F(GraphTest, InsertNode) {
  Function::Editor editor(function());
  auto const block2 = block_at(1);
  auto const block4 = block_at(3);

  // Move |block2| before |block4|.
  editor.RemoveNode(block2);
  editor.InsertNode(block2, block4);
  EXPECT_EQ(
      "{id:1 predecessors:{} successors:{2, 3}}\n"
      "{id:3 predecessors:{1} successors:{4}}\n"
      "{id:2 predecessors:{1} successors:{4}}\n"
      "{id:4 predecessors:{2, 3} successors:{}}\n",
      ToString(function()));
}

TEST_F(GraphTest, OrderedList) {
  EXPECT_EQ("[1, 2, 4, 3]", ToString(function()->ComputePreOrderList()));
  EXPECT_EQ("[4, 2, 3, 1]", ToString(function()->ComputePostOrderList()));
  EXPECT_EQ("[3, 4, 2, 1]", ToString(function()->ComputeReversePreOrderList()));
  EXPECT_EQ("[1, 3, 2, 4]",
            ToString(function()->ComputeReversePostOrderList()));
}

TEST_F(GraphTest, RemoveEdge) {
  Function::Editor editor(function());
  auto const block1 = block_at(0);
  auto const block2 = block_at(1);
  auto const block3 = block_at(2);
  auto const block4 = block_at(3);
  editor.RemoveEdge(block1, block2);
  editor.RemoveEdge(block2, block4);
  EXPECT_EQ(
      "{id:1 predecessors:{} successors:{3}}\n"
      "{id:2 predecessors:{} successors:{}}\n"
      "{id:3 predecessors:{1} successors:{4}}\n"
      "{id:4 predecessors:{3} successors:{}}\n",
      ToString(function()));

  EXPECT_FALSE(block1->HasPredecessor());
  EXPECT_TRUE(block1->HasSuccessor());
  EXPECT_FALSE(block1->HasMoreThanOnePredecessors());

  EXPECT_FALSE(block2->HasPredecessor());
  EXPECT_FALSE(block2->HasSuccessor());
  EXPECT_FALSE(block2->HasMoreThanOnePredecessors());

  EXPECT_TRUE(block3->HasPredecessor());
  EXPECT_TRUE(block3->HasSuccessor());
  EXPECT_FALSE(block3->HasMoreThanOnePredecessors());

  EXPECT_TRUE(block4->HasPredecessor());
  EXPECT_FALSE(block4->HasSuccessor());
  EXPECT_FALSE(block4->HasMoreThanOnePredecessors());
}

}  // namespace
}  // namespace elang
