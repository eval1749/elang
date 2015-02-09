// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "elang/base/graphs/graph_editor.h"
#include "elang/base/graphs/graph_sorter.h"
#include "elang/base/graphs/graph_test.h"

namespace elang {
namespace {

using testing::Block;
using testing::Function;

//////////////////////////////////////////////////////////////////////
//
// GraphTest
//
class GraphTest : public testing::GraphTestBase {
 protected:
  GraphTest() = default;
  ~GraphTest() = default;

  // ::testing::Test
  void SetUp() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(GraphTest);
};

// Use diamond graph for test cases.
//      1
//     / \
//     2  3
//     \  /
//      4
void GraphTest::SetUp() {
  MakeDiamondGraph();
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
      function()->ToString());
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
  EXPECT_TRUE(block1->HasMoreThanOneSuccessors());
  EXPECT_TRUE(block2->HasPredecessor());
  EXPECT_TRUE(block2->HasSuccessor());
  EXPECT_FALSE(block2->HasMoreThanOnePredecessors());
  EXPECT_FALSE(block2->HasMoreThanOneSuccessors());

  EXPECT_TRUE(block3->HasPredecessor());
  EXPECT_TRUE(block3->HasSuccessor());
  EXPECT_FALSE(block3->HasMoreThanOnePredecessors());
  EXPECT_FALSE(block3->HasMoreThanOneSuccessors());

  EXPECT_TRUE(block4->HasPredecessor());
  EXPECT_FALSE(block4->HasSuccessor());
  EXPECT_TRUE(block4->HasMoreThanOnePredecessors());
  EXPECT_FALSE(block4->HasMoreThanOneSuccessors());
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
      function()->ToString());
}

// Since predecessors and successors are represented by unordered map,
// iterations in sorter don't produce same result.
TEST_F(GraphTest, FLAKY_OrderedList) {
  EXPECT_EQ("[1, 2, 4, 3]",
            ToString(Function::Sorter::SortByPreOrder(function())));
  EXPECT_EQ("[4, 2, 3, 1]",
            ToString(Function::Sorter::SortByPostOrder(function())));
  EXPECT_EQ("[3, 4, 2, 1]",
            ToString(Function::Sorter::SortByReversePreOrder(function())));
  EXPECT_EQ("[1, 3, 2, 4]",
            ToString(Function::Sorter::SortByReversePostOrder(function())));
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
      function()->ToString());

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
