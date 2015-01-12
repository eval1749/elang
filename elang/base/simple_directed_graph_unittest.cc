// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/base/simple_directed_graph.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

class SimpleDirectedGraphTest : public ::testing::Test {
 protected:
  SimpleDirectedGraphTest() = default;

  SimpleDirectedGraph<int>* graph() { return &graph_; }

  // ::testing::Test
  void SetUp() override;

 private:
  SimpleDirectedGraph<int> graph_;
  DISALLOW_COPY_AND_ASSIGN(SimpleDirectedGraphTest);
};

std::vector<int> Sort(const std::vector<int>& vector) {
  auto result = vector;
  std::sort(result.begin(), result.end(),
            [](int a, int b) { return a < b; });
  return result;
}

// Build graph
//      1
//     / \
//     2  3
//     \  /
//      4
void SimpleDirectedGraphTest::SetUp() {
  graph()->AddEdge(1, 2);
  graph()->AddEdge(1, 3);
  graph()->AddEdge(2, 4);
  graph()->AddEdge(3, 4);
}

TEST_F(SimpleDirectedGraphTest, AddEdge) {
  // Vertex 1
  EXPECT_TRUE(graph()->HasOutEdge(1));
  EXPECT_FALSE(graph()->HasInEdge(1));
  EXPECT_TRUE(graph()->HasEdge(1, 2));
  EXPECT_TRUE(graph()->HasEdge(1, 3));
  EXPECT_FALSE(graph()->HasEdge(3, 1)) << "should not have edge 3->1";

  // Vertex 2
  EXPECT_TRUE(graph()->HasOutEdge(2));
  EXPECT_TRUE(graph()->HasInEdge(2));
  EXPECT_TRUE(graph()->HasEdge(2, 4));

  // Vertex 3
  EXPECT_TRUE(graph()->HasOutEdge(3));
  EXPECT_TRUE(graph()->HasInEdge(3));
  EXPECT_TRUE(graph()->HasEdge(3, 4));

  // Vertex 4
  EXPECT_FALSE(graph()->HasOutEdge(4));
  EXPECT_TRUE(graph()->HasInEdge(4));
}

TEST_F(SimpleDirectedGraphTest, GetAllVertices) {
  EXPECT_EQ((std::vector<int>{1, 2, 3, 4}), Sort(graph()->GetAllVertices()));
}

TEST_F(SimpleDirectedGraphTest, GetInEdges) {
  EXPECT_EQ(std::vector<int>{1}, Sort(graph()->GetInEdges(2)));
  EXPECT_EQ((std::vector<int>{2, 3}), Sort(graph()->GetInEdges(4)));
}

TEST_F(SimpleDirectedGraphTest, GetOutEdges) {
  EXPECT_EQ((std::vector<int>{2, 3}), Sort(graph()->GetOutEdges(1)));
  EXPECT_EQ(std::vector<int>{4}, Sort(graph()->GetOutEdges(2)));
}

TEST_F(SimpleDirectedGraphTest, RemoveEdge) {
  graph()->RemoveEdge(1, 2);
  EXPECT_FALSE(graph()->HasInEdge(2));
  EXPECT_TRUE(graph()->HasOutEdge(2));
  EXPECT_FALSE(graph()->HasEdge(1, 2));
  EXPECT_TRUE(graph()->HasEdge(1, 3));
  EXPECT_TRUE(graph()->HasEdge(2, 4));
}

}  // namespace
}  // namespace elang
