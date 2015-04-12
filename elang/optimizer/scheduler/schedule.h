// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_
#define ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/base/zone_owner.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class Function;
class Node;

//////////////////////////////////////////////////////////////////////
//
// Schedule
//
class Schedule final : public ZoneOwner {
 public:
  explicit Schedule(Function* function);
  ~Schedule();

  Function* function() const { return function_; }

  BasicBlock* BlockOf(Node* node) const;

 private:
  friend class CfgBuilder;

  Function* const function_;

  // Mapping from |Node| node to |BasicBlock|
  std::unordered_map<Node*, BasicBlock*> block_map_;

  DISALLOW_COPY_AND_ASSIGN(Schedule);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_
