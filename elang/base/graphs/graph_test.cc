// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>

#include "elang/base/graphs/graph_editor.h"
#include "elang/base/graphs/graph_test.h"

namespace elang {
namespace testing {

// Function
std::string Function::ToString() const {
  std::stringstream ostream;
  ostream << *this;
  return ostream.str();
}

// PrintableBlocks
PrintableBlocks::PrintableBlocks(const ZoneUnorderedSet<Block*>& block_set)
    : blocks(block_set.begin(), block_set.end()) {
  std::sort(blocks.begin(), blocks.end(),
            [](Block* a, Block* b) { return a->id() < b->id(); });
}

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

Block* GraphTestBase::NewBlock(int id) {
  auto const block = new (zone()) Block(zone(), id);
  blocks_.push_back(block);
  return block;
}

std::string GraphTestBase::ToString(const OrderedList<Block*>& list) {
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

// Build graph
//      1
//     / \
//     2  3
//     \  /
//      4
void GraphTestBase::MakeDiamondGraph() {
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

}  // namespace testing
}  // namespace elang
