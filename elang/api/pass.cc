// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/api/pass.h"

#include "base/logging.h"
#include "elang/api/pass_controller.h"

namespace elang {
namespace api {

//////////////////////////////////////////////////////////////////////
//
// Pass::RunScope
//
Pass::RunScope::RunScope(Pass* pass) : pass_(pass), stop_(!pass_->StartPass()) {
}

Pass::RunScope::~RunScope() {
  pass_->EndPass();
}

//////////////////////////////////////////////////////////////////////
//
// Pass
//
Pass::Pass(PassController* pass_controller)
    : pass_controller_(pass_controller) {
}

Pass::~Pass() {
}

void Pass::DumpAfterPass(const PassDumpContext& context) {
}

void Pass::DumpBeforePass(const PassDumpContext& context) {
}

void Pass::EndPass() {
  pass_controller_->DidEndPass(this);
}

bool Pass::StartPass() {
  return pass_controller_->DidStartPass(this);
}

}  // namespace api
}  // namespace elang
