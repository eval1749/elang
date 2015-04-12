// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/scheduler/schedule.h"

#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Schedule
//
Schedule::Schedule(Function* function) : function_(function) {
}

Schedule::~Schedule() {
}

BasicBlock* Schedule::BlockOf(Node* node) const {
  DCHECK(node->IsBlockStart() || node->IsBlockEnd());
  auto const it = block_map_.find(node);
  DCHECK(it != block_map_.end());
  return it->second;
}

}  // namespace optimizer
}  // namespace elang
