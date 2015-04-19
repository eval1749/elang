// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/analysis/data_flow_solver.h"
#include "elang/base/analysis/liveness_builder.h"
#include "elang/base/graphs/graph.h"
#include "elang/base/graphs/graph_editor.h"
#include "elang/base/zone_owner.h"
#include "gtest/gtest.h"

namespace elang {

namespace {

class Block;
class Variable;

// The graph.
class Function : public Graph<Function, Block> {
 public:
  typedef DataFlowSolver<Function, Variable*> DataFlowSolver;
  typedef LivenessCollection<Block*, Variable*> Liveness;

  Function() = default;
  ~Function() = default;

  Variable* variable_at(int index) const { return variables_[index]; }
  void AddVariable(Variable* variable) { variables_.push_back(variable); }

 private:
  std::vector<Variable*> variables_;
};

// The graph node.
class Block : public Graph<Function, Block>::GraphNodeBase,
              public ZoneAllocated {
 public:
  Block(Zone* zone, int id) : GraphNodeBase(zone), id_(id) {}
  ~Block() = default;

  int id() const { return id_; }

 private:
  int id_;
};

class Variable : public ZoneAllocated {
 public:
  explicit Variable(base::StringPiece name) : name_(name) {}

  base::StringPiece name() const { return name_; }

 private:
  base::StringPiece name_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

std::ostream& operator<<(std::ostream& ostream, const Block& block) {
  return ostream << "block" << block.id();
}

std::string ToString(const Function& function, const BitSet& bit_set) {
  std::stringstream ostream;
  ostream << "{";
  auto separator = "";
  for (auto member : bit_set) {
    auto const variable = function.variable_at(member);
    ostream << separator << variable->name();
    separator = ", ";
  }
  ostream << "}";
  return ostream.str();
}

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
    ostream << separator << *block;
    separator = ", ";
  }
  ostream << "}";
  return ostream;
}

std::string ToString(const Function& function,
                     const Function::Liveness& collection) {
  std::stringstream ostream;
  for (auto const block : function.nodes()) {
    auto& liveness = collection.LivenessOf(block);
    ostream << *block << ":{"
            << "in:" << ToString(function, liveness.in()) << ", "
            << "out:" << ToString(function, liveness.out()) << ", "
            << "kill:" << ToString(function, liveness.kill()) << ", "
            << "succ:" << PrintableBlocks(block->successors()) << std::endl;
  }
  return ostream.str();
}

//////////////////////////////////////////////////////////////////////
//
// DataFlowSolverTest
//
class DataFlowSolverTest : public ::testing::Test, public ZoneOwner {
 protected:
  DataFlowSolverTest() = default;
  ~DataFlowSolverTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(DataFlowSolverTest);
};

// Test cases...

//      B0---------+    B0 -> B1, B6    a, b, c <- param[0...2]
//      |          |
//      B1<------+ |    B1 -> B2, B4
//      |        | |
//   +->B2-->B5  | |    B2 -> B3, B5    use(c)
//   |  |    |   | |
//   +--B3<--+   | |    B3 -> B4, B2    kill(%2)
//      |        | |
//      B4<------+ |    B4 -> B1, B6    use(b)
//      |          |    B5 -> B3        use(c)
//      B6<--------+    B6              use(a)
//
TEST_F(DataFlowSolverTest, Basic) {
  Function function;
  Function::Editor editor(&function);

  LivenessBuilder<Block*, Variable*> builder;
  auto const var_a = new (zone()) Variable("a");
  auto const var_b = new (zone()) Variable("b");
  auto const var_c = new (zone()) Variable("c");
  function.AddVariable(var_a);
  function.AddVariable(var_b);
  function.AddVariable(var_c);
  builder.AddVariable(var_a);
  builder.AddVariable(var_b);
  builder.AddVariable(var_c);

  std::array<Block*, 7> blocks;
  auto id = 0;
  for (auto& ref : blocks) {
    auto const block = new (zone()) Block(zone(), id);
    ++id;
    ref = block;
    editor.AppendNode(block);
    builder.AddNode(block);
  }

  editor.AddEdge(blocks[0], blocks[1]);
  editor.AddEdge(blocks[0], blocks[6]);
  builder.MarkKill(builder.Edit(blocks[0]), var_a);
  builder.MarkKill(builder.Edit(blocks[0]), var_b);
  builder.MarkKill(builder.Edit(blocks[0]), var_c);

  editor.AddEdge(blocks[1], blocks[2]);
  editor.AddEdge(blocks[1], blocks[4]);

  editor.AddEdge(blocks[2], blocks[5]);
  editor.AddEdge(blocks[2], blocks[3]);
  builder.MarkUse(builder.Edit(blocks[2]), var_b);

  editor.AddEdge(blocks[3], blocks[2]);
  editor.AddEdge(blocks[3], blocks[4]);
  builder.MarkKill(builder.Edit(blocks[3]), var_c);

  editor.AddEdge(blocks[4], blocks[1]);
  editor.AddEdge(blocks[4], blocks[6]);
  builder.MarkUse(builder.Edit(blocks[4]), var_b);

  editor.AddEdge(blocks[5], blocks[3]);
  builder.MarkUse(builder.Edit(blocks[5]), var_c);

  builder.MarkUse(builder.Edit(blocks[6]), var_a);

  auto const collection = builder.Finish();
  Function::DataFlowSolver(&function, collection.get()).SolveBackward();
  EXPECT_EQ(
      "block0:{in:{}, out:{a, b, c}, kill:{a, b, c}, succ:{block1, block6}\n"
      "block1:{in:{a, b, c}, out:{a, b, c}, kill:{}, succ:{block2, block4}\n"
      "block2:{in:{a, b, c}, out:{a, b, c}, kill:{}, succ:{block3, block5}\n"
      "block3:{in:{a, b}, out:{a, b, c}, kill:{c}, succ:{block2, block4}\n"
      "block4:{in:{a, b, c}, out:{a, b, c}, kill:{}, succ:{block1, block6}\n"
      "block5:{in:{a, b, c}, out:{a, b}, kill:{}, succ:{block3}\n"
      "block6:{in:{a}, out:{}, kill:{}, succ:{}\n",
      ToString(function, *collection));
}

}  // namespace
}  // namespace elang
