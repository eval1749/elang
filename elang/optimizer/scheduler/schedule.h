// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_
#define ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_

#include <iosfwd>

#include "base/macros.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_vector.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class Function;
class Node;
class ScheduleEdtior;

//////////////////////////////////////////////////////////////////////
//
// Schedule
//
class ELANG_OPTIMIZER_EXPORT Schedule final : public ZoneOwner {
 public:
  explicit Schedule(Function* function);
  ~Schedule();

  Function* function() const { return function_; }
  const ZoneVector<Node*>& nodes() const { return nodes_; }

 private:
  friend class ScheduleEditor;

  Function* const function_;
  ZoneVector<Node*> nodes_;

  DISALLOW_COPY_AND_ASSIGN(Schedule);
};

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const Schedule& schedule);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_H_
