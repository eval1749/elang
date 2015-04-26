// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/basic_block.h"

#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
BasicBlock::BasicBlock(Zone* zone) : GraphNodeBase(zone), nodes_(zone) {
  nodes_.reserve(2);
}

std::ostream& operator<<(std::ostream& ostream, const BasicBlock* block) {
  if (!block)
    return ostream << "nil";
  return ostream << *block;
}

std::ostream& operator<<(std::ostream& ostream, const BasicBlock& block) {
  ostream << "block";
  if (block.nodes().empty())
    return ostream << "@" << static_cast<const void*>(&block);
  return ostream << block.nodes().front()->id();
}

}  // namespace optimizer
}  // namespace elang
