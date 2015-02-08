// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/pass.h"

namespace elang {
namespace lir {

// Pass
Pass::Pass(Factory* factory) : FactoryUser(factory) {
}

Pass::~Pass() {
}

// FuncitonPass
FunctionPass::FunctionPass(Factory* factory, Function* function)
    : Pass(factory), function_(function) {
}

FunctionPass::~FunctionPass() {
}

// Pass
void FunctionPass::Run() {
  RunOnFunction();
}

}  // namespace lir
}  // namespace elang
