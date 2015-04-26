// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#include "base/logging.h"
#include "elang/base/tree_algorithm.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

class MyNode {
 public:
  explicit MyNode(std::string name) : name_(name), parent_(nullptr) {}
  ~MyNode() = default;

  std::string name() const { return name_; }
  MyNode* parent() const { return parent_; }

  void AppendChild(MyNode* new_child);
  void RemoveChild(MyNode* old_child);

 private:
  std::vector<MyNode*> children_;
  std::string name_;
  MyNode* parent_;

  DISALLOW_COPY_AND_ASSIGN(MyNode);
};

void MyNode::AppendChild(MyNode* new_child) {
  if (auto const parent = new_child->parent_) {
    if (parent == this)
      return;
    parent->RemoveChild(new_child);
  }
  new_child->parent_ = this;
  children_.push_back(new_child);
}

void MyNode::RemoveChild(MyNode* old_child) {
  DCHECK_EQ(this, old_child->parent_);
  auto const it = std::find(children_.begin(), children_.end(), old_child);
  DCHECK(it != children_.end());
  children_.erase(it);
}

std::ostream& operator<<(std::ostream& ostream, const MyNode& node) {
  return ostream << node.name();
}

std::ostream& operator<<(std::ostream& ostream, const MyNode* node) {
  if (!node)
    return ostream << "(null)";
  return ostream << *node;
}

class MyNodeTraversal {
 public:
  using Node = MyNode;
  static int DepthOf(const Node* node);
  static Node* ParentOf(const Node* node) { return node->parent(); }
};

int MyNodeTraversal::DepthOf(const Node* node) {
  auto depth = 0;
  for (auto runner = node; runner; runner = ParentOf(runner))
    ++depth;
  return depth;
}

using MyTreeAlgorithm = TreeAlgorithm<MyNodeTraversal>;

}  // namespace

TEST(TreeAlgorithmTest, CommonAncestorOf) {
  // Build following tree:
  //   A
  //  B C
  //    D E
  MyNode nodeA("A");
  MyNode nodeB("B");
  MyNode nodeC("C");
  MyNode nodeD("D");
  MyNode nodeE("E");
  MyNode nodeF("F");
  nodeA.AppendChild(&nodeB);
  nodeA.AppendChild(&nodeC);
  nodeC.AppendChild(&nodeD);
  nodeC.AppendChild(&nodeE);

  EXPECT_EQ(&nodeA, MyTreeAlgorithm::CommonAncestorOf(&nodeA, &nodeA));
  EXPECT_EQ(&nodeA, MyTreeAlgorithm::CommonAncestorOf(&nodeA, &nodeB));
  EXPECT_EQ(&nodeA, MyTreeAlgorithm::CommonAncestorOf(&nodeB, &nodeC));
  EXPECT_EQ(&nodeC, MyTreeAlgorithm::CommonAncestorOf(&nodeC, &nodeE));
  EXPECT_EQ(&nodeC, MyTreeAlgorithm::CommonAncestorOf(&nodeD, &nodeE));
  EXPECT_EQ(nullptr, MyTreeAlgorithm::CommonAncestorOf(&nodeD, &nodeF));
}

}  // namespace elang
