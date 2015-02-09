// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_GRAPHS_GRAPH_TEST_H_
#define ELANG_BASE_GRAPHS_GRAPH_TEST_H_

#include <ostream>
#include <string>
#include <vector>

#include "elang/base/graphs/graph.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "gtest/gtest.h"

namespace elang {
namespace testing {

class Block;

//////////////////////////////////////////////////////////////////////
//
// Represents graph owner of |Blocks|.
//
class Function : public Graph<Function, Block> {
 public:
  Function() = default;
  ~Function() = default;

  std::string ToString() const;
};

//////////////////////////////////////////////////////////////////////
//
// Represents graph node.
//
class Block : public Graph<Function, Block>::Node, public ZoneAllocated {
 public:
  Block(Zone* zone, int id) : Node(zone), id_(id) {}
  ~Block() = default;

  int id() const { return id_; }

  std::string ToString() const;

 private:
  int id_;
};

//////////////////////////////////////////////////////////////////////
//
// Helper class for printing set of |Block|.
//
struct PrintableBlocks {
  std::vector<Block*> blocks;

  explicit PrintableBlocks(const ZoneUnorderedSet<Block*>& block_set);
};

std::ostream& operator<<(std::ostream& ostream, const PrintableBlocks& blocks);
std::ostream& operator<<(std::ostream& ostream, const Block& block);
std::ostream& operator<<(std::ostream& ostream, const Function& function);

//////////////////////////////////////////////////////////////////////
//
// GraphTestBase
//
class GraphTestBase : public ::testing::Test, public ZoneOwner {
 protected:
  GraphTestBase() = default;
  ~GraphTestBase() = default;

  Block* block_at(int index) { return blocks_[index]; }
  Function* function() { return &function_; }

  void MakeDiamondGraph();
  Block* NewBlock(int id);
  static std::string ToString(const OrderedList<Block*>& list);

 private:
  Function function_;
  std::vector<Block*> blocks_;

  DISALLOW_COPY_AND_ASSIGN(GraphTestBase);
};

}  // namespace testing
}  // namespace elang

namespace std {
template <>
struct hash<elang::testing::Block> {
  size_t operator()(const elang::testing::Block& block) const {
    return static_cast<size_t>(block.id());
  }
};
}  // namespace std

#endif  // ELANG_BASE_GRAPHS_GRAPH_TEST_H_
