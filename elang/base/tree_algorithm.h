// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_TREE_ALGORITHM_H_
#define ELANG_BASE_TREE_ALGORITHM_H_

namespace elang {

// |Traversal| should have following members:
//  typename Node
//  Node* parent(const Node* node)

template <typename Traversal>
class TreeAlgorithm : public Traversal {
 public:
  using Node = typename Traversal::Node;

  // Returns least common ancestor of |nodeA| and |nodeB|.
  static Node* CommonAncestorOf(const Node* nodeA, const Node* nodeB);
};

template <typename Traversal>
typename Traversal::Node* TreeAlgorithm<Traversal>::CommonAncestorOf(
    const Node* nodeA,
    const Node* nodeB) {
  if (nodeA == nodeB)
    return const_cast<Node*>(nodeA);
  auto depthA = 0;
  for (auto runner = nodeA; runner; runner = Traversal::parentOf(runner)) {
    if (runner == nodeB)
      return const_cast<Node*>(runner);
    ++depthA;
  }
  auto depthB = 0;
  for (auto runner = nodeB; runner; runner = Traversal::parentOf(runner)) {
    if (runner == nodeA)
      return const_cast<Node*>(runner);
    ++depthB;
  }
  auto runnerA = nodeA;
  auto runnerB = nodeB;
  if (depthA > depthB) {
    for (auto depth = depthA; depth > depthB; --depth)
      runnerA = Traversal::parentOf(runnerA);
  } else if (depthB > depthA) {
    for (auto depth = depthB; depth > depthB; --depth)
      runnerB = Traversal::parentOf(runnerB);
  }
  while (runnerA) {
    if (runnerA == runnerB)
      return const_cast<Node*>(runnerA);
    runnerA = Traversal::parentOf(runnerA);
    runnerB = Traversal::parentOf(runnerB);
  }
  return nullptr;
}

}  // namespace elang

#endif  // ELANG_BASE_TREE_ALGORITHM_H_
