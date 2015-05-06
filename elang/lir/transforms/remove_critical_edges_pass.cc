// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/transforms/remove_critical_edges_pass.h"

#include "base/logging.h"
#include "elang/base/ordered_list.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

namespace {
bool HasBackEdge(const OrderedList<BasicBlock*>& blocks, BasicBlock* block) {
  auto const position = blocks.position_of(block);
  for (auto const predecessor : block->predecessors()) {
    if (blocks.position_of(predecessor) >= position)
      return true;
  }
  return false;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RemoveCriticalEdgesPass
//
RemoveCriticalEdgesPass::RemoveCriticalEdgesPass(base::StringPiece name,
                                                 Editor* editor)
    : FunctionPass(name, editor) {
}

RemoveCriticalEdgesPass::~RemoveCriticalEdgesPass() {
}

// Removes critical edges
//  - An edge to a block which have phi instruction and predecessor having
//    more than one predecessors, or
//  - A back edge from a block having more than one successors.
//
// Example of critical edge:
//  block10:
//    br block30
//  block20:
//    // Out: {block30, block21}
//    br %flag20, block30, block21
//  block30:
//    // In: {block10, block20}
//    phi %r30 = block10: %r10, block20: %r20
//
//  (block20 => block30) is a critical edge.
//
//  block10:
//    // Out: {block30}
//    br block30
//  block20:
//    // Out: {block30, block21}
//    br %flag20, block25, block21
//  block25:        // This block is inserted for removing a critical edge.
//    // In: {block20}
//    // Out: {block 30}
//    br block25
//  block30:
//    // In: {block10, block25}
//    phi %r30 = block10: %r10, block25: %r20
//
// Note: TODO(eval1749) We don't need to remove critical edges if they are back
// edges and phi operands aren't live out in other successors.
void RemoveCriticalEdgesPass::RunOnFunction() {
  // Since we modify predecessors of block, we can't use |predecessors()|
  // during inserting new block.
  struct WorkItem {
    BasicBlock* block;
    BasicBlock* predecessor;
  };
  std::vector<WorkItem> items;

  // Collection phase
  auto const& blocks = editor()->ReversePostOrderList();
  for (auto const block : blocks) {
    if (block->phi_instructions().empty() && !HasBackEdge(blocks, block))
      continue;
    for (auto const predecessor : block->predecessors()) {
      if (predecessor->HasMoreThanOneSuccessor()) {
        items.push_back({block, predecessor});
        continue;
      }
    }
  }

  // Rewriting phase
  for (auto const item : items) {
    // Insert new block after |predecessor|.
    auto const new_block = editor()->NewBasicBlock(item.predecessor->next());

    // |new_block| to |phi_block|
    editor()->Edit(new_block);
    editor()->SetJump(item.block);
    editor()->Commit();

    // Redirect edge |predecessor| => |phi_block| to |new_block| => |phi_block|
    editor()->Edit(item.predecessor);
    auto const last = item.predecessor->last_instruction();
    auto position = 0;
    for (auto const target : last->block_operands()) {
      if (target == item.block)
        editor()->SetBlockOperand(last, position, new_block);
      ++position;
    }
    editor()->Commit();

    // Update phi inputs
    editor()->Edit(item.block);
    editor()->ReplacePhiInputs(new_block, item.predecessor);
    editor()->Commit();
  }
}

}  // namespace lir
}  // namespace elang
