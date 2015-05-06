// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "elang/lir/pipeline.h"

#include "elang/api/pass.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/emitters/code_emitter.h"
#include "elang/lir/transforms/clean_pass.h"
#include "elang/lir/transforms/lowering_x64_pass.h"
#include "elang/lir/transforms/remove_critical_edges_pass.h"
#include "elang/lir/transforms/register_allocation_pass.h"

namespace elang {
namespace lir {

namespace {

template <typename Pass>
bool RunPass(base::StringPiece name, Editor* editor) {
  return Pass(name, editor).Run();
}

typedef bool PassEntry(base::StringPiece name, Editor* editor);

struct PassInfo {
  const char* name;
  PassEntry* entry;
};

//////////////////////////////////////////////////////////////////////
//
// CodeEmitterPass
//
class CodeEmitterPass final : public api::Pass {
 public:
  CodeEmitterPass(Factory* factory,
                  api::MachineCodeBuilder* builder,
                  Function* function);
  ~CodeEmitterPass() = default;

  bool Run();

 private:
  // api::Pass
  base::StringPiece name() const final { return "emit"; }

  api::MachineCodeBuilder* const builder_;
  Factory* const factory_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(CodeEmitterPass);
};

CodeEmitterPass::CodeEmitterPass(Factory* factory,
                                 api::MachineCodeBuilder* builder,
                                 Function* function)
    : api::Pass(factory->pass_controller()),
      builder_(builder),
      factory_(factory),
      function_(function) {
}

bool CodeEmitterPass::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return false;
  CodeEmitter(factory_, builder_).Process(function_);
  return factory_->errors().empty();
}

//////////////////////////////////////////////////////////////////////
//
// kPassList
//
PassInfo const kPassList[] = {
    {"lowering", &RunPass<LoweringX64Pass>},
    {"critical_edge", &RunPass<RemoveCriticalEdgesPass>},
    {"ra", &RunPass<RegisterAssignmentsPass>},
    {"final_clean", &RunPass<CleanPass>},
};

}  // namespace

Pipeline::Pipeline(Factory* factory,
                   api::MachineCodeBuilder* builder,
                   Function* function)
    : builder_(builder), factory_(factory), function_(function) {
}

Pipeline::~Pipeline() {
}

bool Pipeline::Run() {
  Editor editor(factory_, function_);
  for (auto it = std::begin(kPassList); it != std::end(kPassList); ++it) {
    if (!it->entry(it->name, &editor))
      return false;
    if (!factory_->errors().empty())
      return false;
  }

  return CodeEmitterPass(factory_, builder_, function_).Run();
}

}  // namespace lir
}  // namespace elang
