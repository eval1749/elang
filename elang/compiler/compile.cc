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
bool RunPass(NameResolver* name_resolver) {
  Pass(name_resolver).Run();
  return name_resolver->session()->errors().empty();
}
}  // namespace

bool CompilationSession::Compile(NameResolver* name_resolver,
                                 hir::Factory* factory) {
  if (!errors().empty())
    return false;
  if (!RunPass<NamespaceAnalyzer>(name_resolver))
    return false;
  if (!RunPass<ClassAnalyzer>(name_resolver))
    return false;
  if (!RunPass<MethodAnalyzer>(name_resolver))
    return false;

  Zone zone;
  VariableAnalyzer variable_analyzer(&zone);
  CodeGenerator(this, factory, &variable_analyzer).Run();
  if (!errors().empty())
    return false;
  auto const variable_usages = variable_analyzer.Analyze();
  for (auto const method_function : function_map_) {
    auto const function = method_function.second;
    hir::Editor editor(factory, function);
    CfgToSsaConverter(&editor, variable_usages).Run();
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
