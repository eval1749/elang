// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/api/pass_observer.h"

namespace elang {
namespace api {

//////////////////////////////////////////////////////////////////////
//
// PassObserver
//
PassObserver::PassObserver() {
}

PassObserver::~PassObserver() {
}

void PassObserver::DidEndPass(Pass* pass) {
}

void PassObserver::DidStartPass(Pass* pass) {
}

}  // namespace api
}  // namespace elang
