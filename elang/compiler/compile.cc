// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/compilation_session.h"

#include "elang/compiler/analysis/class_analyzer.h"
#include "elang/compiler/analysis/method_analyzer.h"
#include "elang/compiler/analysis/namespace_analyzer.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/cg/code_generator.h"
#include "elang/compiler/cg/cfg_to_ssa_converter.h"
#include "elang/compiler/cg/variable_analyzer.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/values.h"

namespace elang {
namespace compiler {

namespace {
template <typename Pass>
bool Run(NameResolver* name_resolver) {
  Pass pass(name_resolver);
  return pass.Run();
}

}  // namespace

bool CompilationSession::Compile(NameResolver* name_resolver,
                                 hir::Factory* factory) {
  if (!errors().empty())
    return false;
  if (!Run<NamespaceAnalyzer>(name_resolver))
    return false;
  if (!Run<ClassAnalyzer>(name_resolver))
    return false;
  if (!Run<MethodAnalyzer>(name_resolver))
    return false;

  Zone zone;
  VariableAnalyzer variable_analyzer(&zone);
  {
    CodeGenerator generator(this, factory, &variable_analyzer);
    if (!generator.Run())
      return false;
  }
  auto const variable_usages = variable_analyzer.Analyze();
  for (auto const method_function : function_map_) {
    auto const function = method_function.second;
    hir::Editor editor(factory, function);
    CfgToSsaConverter pass(&editor, variable_usages);
    pass.Run();
  }
  return errors().empty();
}

hir::Function* CompilationSession::FunctionOf(ast::Method* method) {
  auto const it = function_map_.find(method);
  return it == function_map_.end() ? nullptr : it->second;
}

void CompilationSession::RegisterFunction(ast::Method* method,
                                          hir::Function* function) {
  DCHECK(!function_map_.count(method));
  function_map_[method] = function;
}

}  // namespace compiler
}  // namespace elang
