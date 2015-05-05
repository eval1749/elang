// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "elang/lir/pipeline.h"

#include "elang/api/pass.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/emitters/code_emitter.h"
#include "elang/lir/formatters/text_formatter.h"
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

struct PassInfo {
  const char* name;
  PassEntry* entry;
};

//////////////////////////////////////////////////////////////////////
//
// PassWrapper
//
class PassWrapper final : public api::Pass {
 public:
  PassWrapper(const PassInfo& info, Editor* editor);
  ~PassWrapper() = default;

  bool Run();

 private:
  // api::Pass
  base::StringPiece name() const { return info_.name; }
  void DumpAfterPass(const api::PassDumpContext& context) final;
  void DumpBeforePass(const api::PassDumpContext& context) final;

  Editor* const editor_;
  const PassInfo& info_;

  DISALLOW_COPY_AND_ASSIGN(PassWrapper);
};

PassWrapper::PassWrapper(const PassInfo& info, Editor* editor)
    : api::Pass(editor->factory()->pass_controller()),
      editor_(editor),
      info_(info) {
}

bool PassWrapper::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return false;
  info_.entry(editor_);
  return true;
}

// api::Pass
void PassWrapper::DumpAfterPass(const api::PassDumpContext& context) {
  TextFormatter formatter(editor_->factory()->literals(), context.ostream);
  formatter.FormatFunction(editor_->function());
}

void PassWrapper::DumpBeforePass(const api::PassDumpContext& context) {
  DumpAfterPass(context);
}

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
    if (!PassWrapper(*it, &editor).Run())
      return false;
    if (!factory_->errors().empty())
      return false;
  }

  return CodeEmitterPass(factory_, builder_, function_).Run();
}

}  // namespace lir
}  // namespace elang
