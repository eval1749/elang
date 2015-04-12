// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_SCHEDULER_H_
#define ELANG_OPTIMIZER_SCHEDULER_SCHEDULER_H_

#include "base/macros.h"

namespace elang {
namespace optimizer {

class Schedule;

//////////////////////////////////////////////////////////////////////
//
// Scheduler
//
class Scheduler final {
 public:
  explicit Scheduler(Schedule* schedule);
  ~Scheduler();

  void Run();

 private:
  Schedule* const schedule_;

  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_SCHEDULER_H_
