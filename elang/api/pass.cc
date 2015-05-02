// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/api/pass.h"

#include "base/logging.h"
#include "elang/api/pass_observer.h"

namespace elang {
namespace api {

//////////////////////////////////////////////////////////////////////
//
// Pass::RunScope
//
Pass::RunScope::RunScope(Pass* pass) : pass_(pass) {
  pass_->StartPass();
}

Pass::RunScope::~RunScope() {
  pass_->EndPass();
}

//////////////////////////////////////////////////////////////////////
//
// Pass
//
Pass::Pass(PassObserver* observer) : observer_(observer) {
}

Pass::~Pass() {
}

base::TimeDelta Pass::duration() const {
  DCHECK(start_at_ != base::Time());
  DCHECK(end_at_ != base::Time());
  return end_at_ - start_at_;
}

void Pass::DumpAfterPass(const PassDumpContext& context) {
}

void Pass::DumpBeforePass(const PassDumpContext& context) {
}

void Pass::EndPass() {
  DCHECK(start_at_ != base::Time());
  DCHECK(end_at_ == base::Time());
  end_at_ = base::Time::Now();
  observer_->DidEndPass(this);
}

void Pass::StartPass() {
  DCHECK(start_at_ == base::Time());
  DCHECK(end_at_ == base::Time());
  start_at_ = base::Time::Now();
  observer_->DidStartPass(this);
}

}  // namespace api
}  // namespace elang
