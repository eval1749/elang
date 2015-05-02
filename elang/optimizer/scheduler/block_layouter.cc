// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <queue>
#include <unordered_set>
#include <vector>

#include "elang/optimizer/scheduler/block_layouter.h"

#include "base/logging.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/scheduler/basic_block.h"
#include "elang/optimizer/scheduler/control_flow_graph.h"
#include "elang/optimizer/scheduler/static_predictor.h"

namespace elang {
namespace optimizer {

namespace {

//////////////////////////////////////////////////////////////////////
//
// Edge
//
struct Edge {
  const BasicBlock* from;
  const BasicBlock* to;
  double frequency;
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BlockLayouter::Chain
//
class BlockLayouter::Chain final : public ZoneAllocated {
 public:
  Chain(Zone* zone, const BasicBlock* block, size_t priority);
  ~Chain() = delete;

  BasicBlock* back() const { return blocks_.back(); }
  const ZoneVector<BasicBlock*> blocks() const { return blocks_; }
  BasicBlock* front() const { return blocks_.front(); }
  size_t priority() const { return priority_; }

  bool operator<(const Chain& other) const {
    return priority_ > other.priority_;
  }

  void Append(const Chain* other, size_t priority);

 private:
  // Small number is higher priority.
  size_t priority_;
  ZoneVector<BasicBlock*> blocks_;

  DISALLOW_COPY_AND_ASSIGN(Chain);
};

BlockLayouter::Chain::Chain(Zone* zone,
                            const BasicBlock* block,
                            size_t priority)
    : blocks_(zone), priority_(0) {
  blocks_.push_back(const_cast<BasicBlock*>(block));
}

void BlockLayouter::Chain::Append(const Chain* other, size_t priority) {
  priority_ = std::min(priority_, std::min(other->priority_, priority));
  blocks_.insert(blocks_.end(), other->blocks_.begin(), other->blocks_.end());
}

//////////////////////////////////////////////////////////////////////
//
// BlockLayouter
//
BlockLayouter::BlockLayouter(api::PassObserver* observer,
                             ScheduleEditor* editor,
                             const EdgeProfile* edge_map)
    : Pass(observer), ScheduleEditor::User(editor), edge_map_(edge_map) {
}

BlockLayouter::~BlockLayouter() {
}

void BlockLayouter::BuildChain() {
  std::unordered_set<const BasicBlock*> blocks;
  std::vector<Edge> edges;
  edges.reserve(edge_map_->all_edges().size());
  for (auto pair : edge_map_->all_edges()) {
    auto const from = pair.first.first;
    auto const to = pair.first.second;
    edges.push_back({from, to, pair.second});
    blocks.insert(from);
    blocks.insert(to);
  }
  std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
    if (a.frequency == b.frequency)
      return a.from->id() < b.from->id();
    return a.frequency > b.frequency;
  });

  // Make each block a degenerate chain and set its priority to number of
  // blocks.
  for (auto const block : blocks) {
    auto const chain = new (zone()) Chain(zone(), block, blocks.size());
    chain_map_.insert(std::make_pair(block, chain));
  }

  // Fall edge E = x -> y in CFG, in order by decreasing frequency.
  size_t priority = 0;
  for (auto const& edge : edges) {
    auto const tail = edge.from;
    auto const chain_tail = ChainOf(tail);
    if (chain_tail->back() != tail)
      continue;
    auto const head = edge.to;
    auto const chain_head = ChainOf(head);
    if (chain_head == chain_tail)
      continue;
    if (chain_head->front() != head)
      continue;
    for (auto const block : chain_head->blocks()) {
      auto const it = chain_map_.find(block);
      DCHECK(it != chain_map_.end());
      chain_map_.erase(it);
      chain_map_.insert(std::make_pair(block, chain_tail));
    }
    chain_tail->Append(chain_head, priority);
    ++priority;
  }
}

BlockLayouter::Chain* BlockLayouter::ChainOf(const BasicBlock* block) const {
  auto const it = chain_map_.find(block);
  DCHECK(it != chain_map_.end()) << *block->first_node();
  return it->second;
}

// Intuitions:
//  * Entry node first
//  * Tries to make edge from chain X to chain Y a forward branch
//    - Predicted as take on target machine (forward branch isn't taken)
//    - Edge remains only if it is lower probability choice
std::vector<BasicBlock*> BlockLayouter::Layout() {
  std::unordered_set<Chain*> placed;
  std::vector<BasicBlock*> blocks;
  std::priority_queue<Chain*> work_list;
  auto const entry_block = control_flow_graph()->first_node();
  auto const exit_block = control_flow_graph()->last_node();
  work_list.push(ChainOf(entry_block));
  while (!work_list.empty()) {
    // Pick the chain C with lowest priority from |work_list|.
    auto const chain = work_list.top();
    work_list.pop();
    placed.insert(chain);
    // Place it next in the code
    for (auto const block : chain->blocks()) {
      if (block != exit_block)
        blocks.push_back(block);
      for (auto const use_edge : block->last_node()->use_edges()) {
        auto const successor = BlockOf(use_edge->from());
        auto const chain = ChainOf(successor);
        if (placed.count(chain))
          continue;
        work_list.push(chain);
      }
    }
  }
  DCHECK_EQ(entry_block, blocks.front());
  DCHECK(std::find(blocks.begin(), blocks.end(), exit_block) == blocks.end());
  blocks.push_back(exit_block);
  return blocks;
}

std::vector<BasicBlock*> BlockLayouter::Run() {
  RunScope scope(this);
  BuildChain();
  return Layout();
}

// api::Pass
base::StringPiece BlockLayouter::name() const {
  return "block layouter";
}

}  // namespace optimizer
}  // namespace elang
