// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "elang/optimizer/scheduler/scheduler.h"

#include "base/logging.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/scheduler/cfg_builder.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Scheduler
//
Scheduler::Scheduler(Schedule* schedule) : schedule_(schedule) {
}

Scheduler::~Scheduler() {
}

// The entry point
void Scheduler::Run() {
  CfgBuilder(schedule_).Run();
}

}  // namespace optimizer
}  // namespace elang
