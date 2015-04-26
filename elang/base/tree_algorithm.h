// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_TREE_ALGORITHM_H_
#define ELANG_BASE_TREE_ALGORITHM_H_

namespace elang {

// |Traversal| should have following members:
//  - int DepthOf(const Node* node)
//  - Node* ParentOf(const Node* node)

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
  auto const depthA = Traversal::DepthOf(nodeA);
  auto const depthB = Traversal::DepthOf(nodeB);
  auto runnerA = nodeA;
  auto runnerB = nodeB;
  for (auto depth = depthA; depth > depthB; --depth)
    runnerA = Traversal::ParentOf(runnerA);
  for (auto depth = depthB; depth > depthA; --depth)
    runnerB = Traversal::ParentOf(runnerB);
  while (runnerA && runnerA != runnerB) {
    runnerA = Traversal::ParentOf(runnerA);
    runnerB = Traversal::ParentOf(runnerB);
  }
  return const_cast<Node*>(runnerA);;
}

}  // namespace elang

#endif  // ELANG_BASE_TREE_ALGORITHM_H_
