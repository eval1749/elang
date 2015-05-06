// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/pass.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"

namespace elang {
namespace lir {

// Pass
Pass::Pass(Factory* factory)
    : api::Pass(factory->pass_controller()), FactoryUser(factory) {
}

Pass::~Pass() {
}

// FuncitonPass
FunctionPass::FunctionPass(Editor* editor)
    : Pass(editor->factory()), EditorUser(editor) {
}

FunctionPass::~FunctionPass() {
}

bool FunctionPass::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return false;
  RunOnFunction();
  DCHECK(editor()->Validate());
  return true;
}

// api::Pass
void FunctionPass::DumpAfterPass(const api::PassDumpContext& context) {
  TextFormatter formatter(editor()->factory()->literals(), context.ostream);
  formatter.FormatFunction(editor()->function());
}

void FunctionPass::DumpBeforePass(const api::PassDumpContext& context) {
  DumpAfterPass(context);
}

}  // namespace lir
}  // namespace elang
