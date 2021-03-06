// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_SCHEDULER_H_
#define ELANG_OPTIMIZER_SCHEDULER_SCHEDULER_H_

#include <memory>

#include "base/macros.h"
#include "elang/api/pass.h"
#include "elang/base/analysis/dominator_tree.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class Schedule;
class ScheduleEditor;

//////////////////////////////////////////////////////////////////////
//
// Scheduler
//
class ELANG_OPTIMIZER_EXPORT Scheduler final : public api::Pass {
 public:
  Scheduler(api::PassController* pass_controller, Schedule* schedule);
  ~Scheduler();

  void Run();

 private:
  // api::Pass
  base::StringPiece name() const final;
  void DumpAfterPass(const api::PassDumpContext& context) final;
  void DumpBeforePass(const api::PassDumpContext& context) final;

  std::unique_ptr<ScheduleEditor> editor_;

  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_SCHEDULER_H_
