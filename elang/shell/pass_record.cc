// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/shell/pass_record.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace shell {

//////////////////////////////////////////////////////////////////////
//
// PassRecord
//
PassRecord::PassRecord(size_t depth, base::StringPiece name)
    : depth_(depth), name_(name) {
}

PassRecord::~PassRecord() {
}

base::TimeDelta PassRecord::duration() const {
  DCHECK(start_at_ != base::Time());
  DCHECK(end_at_ != base::Time());
  return end_at_ - start_at_;
}

void PassRecord::EndMetrics() {
  DCHECK(start_at_ != base::Time());
  DCHECK(end_at_ == base::Time());
  end_at_ = base::Time::Now();
}

void PassRecord::StartMetrics() {
  DCHECK(start_at_ == base::Time());
  DCHECK(end_at_ == base::Time());
  start_at_ = base::Time::Now();
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang
