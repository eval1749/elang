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
  PassWrapper(const PassInfo& info,
              api::PassObserver* observer,
              Editor* editor);
  ~PassWrapper() = default;

  void Run();

 private:
  // api::Pass
  base::StringPiece name() const { return info_.name; }
  void DumpAfterPass(const api::PassDumpContext& context) final;
  void DumpBeforePass(const api::PassDumpContext& context) final;

  Editor* const editor_;
  const PassInfo& info_;

  DISALLOW_COPY_AND_ASSIGN(PassWrapper);
};

PassWrapper::PassWrapper(const PassInfo& info,
                         api::PassObserver* observer,
                         Editor* editor)
    : api::Pass(observer), editor_(editor), info_(info) {
}

void PassWrapper::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return;
  info_.entry(editor_);
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
  CodeEmitterPass(api::PassObserver* observer,
                  Factory* factory,
                  api::MachineCodeBuilder* builder,
                  Function* function);
  ~CodeEmitterPass() = default;

  void Run();

 private:
  // api::Pass
  base::StringPiece name() const final { return "emit"; }

  api::MachineCodeBuilder* const builder_;
  Factory* const factory_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(CodeEmitterPass);
};

CodeEmitterPass::CodeEmitterPass(api::PassObserver* observer,
                                 Factory* factory,
                                 api::MachineCodeBuilder* builder,
                                 Function* function)
    : api::Pass(observer),
      builder_(builder),
      factory_(factory),
      function_(function) {
}

void CodeEmitterPass::Run() {
  RunScope scope(this);
  if (scope.IsStop())
    return;
  CodeEmitter(factory_, builder_).Process(function_);
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
  for (auto it = std::begin(kPassList); it != std::end(kPassList); ++it) {
    PassWrapper(*it, observer_, &editor).Run();
    if (!factory_->errors().empty())
      return;
  }

  CodeEmitterPass(observer_, factory_, builder_, function_).Run();
}

}  // namespace lir
}  // namespace elang
