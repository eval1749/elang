// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "elang/lir/pipeline.h"

#include "elang/lir/dump_function_pass.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/emitters/code_emitter.h"
#include "elang/lir/literals.h"
#include "elang/lir/transforms/clean_pass.h"
#include "elang/lir/transforms/lowering_x64_pass.h"
#include "elang/lir/transforms/remove_critical_edges_pass.h"
#include "elang/lir/transforms/register_allocation_pass.h"

namespace elang {
namespace lir {

namespace {

template <typename Pass>
void RunPass(Editor* editor) {
  Pass pass(editor);
  pass.Run();
}

class DumpPass final : public FunctionPass {};

typedef void PassEntry(Editor* editor);

PassEntry* kPasses[] = {
    &RunPass<LoweringX64Pass>,
    &RunPass<RemoveCriticalEdgesPass>,
    &RunPass<DumpFunctionPass>,
    &RunPass<RegisterAssignmentsPass>,
    &RunPass<DumpFunctionPass>,
    &RunPass<CleanPass>,
    &RunPass<DumpFunctionPass>,
};

}  // namespace

Pipeline::Pipeline(Factory* factory,
                   api::PassObserver* observer,
                   api::MachineCodeBuilder* builder,
                   Function* function)
    : builder_(builder),
      factory_(factory),
      function_(function),
      observer_(observer) {
}

Pipeline::~Pipeline() {
}

void Pipeline::Run() {
  Editor editor(factory_, function_);
  for (auto it = std::begin(kPasses); it != std::end(kPasses); ++it) {
    (*it)(&editor);
    if (!factory_->errors().empty())
      return;
  }

  CodeEmitter(factory_, builder_).Process(function_);
}

}  // namespace lir
}  // namespace elang
