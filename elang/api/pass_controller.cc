// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/api/pass_controller.h"

namespace elang {
namespace api {

//////////////////////////////////////////////////////////////////////
//
// PassController
//
PassController::PassController() {
}

PassController::~PassController() {
}

void PassController::DidEndPass(Pass* pass) {
}

bool PassController::DidStartPass(Pass* pass) {
  return true;
}

}  // namespace api
}  // namespace elang
