// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/pass.h"

#include "elang/lir/editor.h"

namespace elang {
namespace lir {

// Pass
Pass::Pass(Factory* factory) : FactoryUser(factory) {
}

Pass::~Pass() {
}

// FuncitonPass
FunctionPass::FunctionPass(Editor* editor)
    : Pass(editor->factory()), EditorUser(editor) {
}

FunctionPass::~FunctionPass() {
}

void FunctionPass::Run() {
  RunOnFunction();
  DCHECK(editor()->Validate());
}

}  // namespace lir
}  // namespace elang
