// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_FORMATTER_H_
#define ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_FORMATTER_H_

#include <ostream>

#include "base/macros.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class Schedule;

struct ELANG_OPTIMIZER_EXPORT FormattedSchedule {
  const Schedule* schedule;
};

ELANG_OPTIMIZER_EXPORT FormattedSchedule AsFormatted(const Schedule& schedule);

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const FormattedSchedule& formatted);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_SCHEDULE_FORMATTER_H_
