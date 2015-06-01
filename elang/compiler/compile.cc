// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "elang/compiler/compilation_session.h"

#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/class_analyzer.h"
#include "elang/compiler/analysis/method_analyzer.h"
#include "elang/compiler/analysis/namespace_analyzer.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/cg/code_generator.h"
#include "elang/compiler/cg/cfg_to_ssa_converter.h"
#include "elang/compiler/cg/variable_analyzer.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/values.h"

namespace elang {
namespace compiler {

namespace {
template <typename Pass>
bool RunPass(NameResolver* name_resolver) {
  Pass(name_resolver).Run();
  return !name_resolver->session()->HasError();
}
}  // namespace

void CompilationSession::Compile(NameResolver* name_resolver,
                                 hir::Factory* factory) {
  if (HasError())
    return;
  if (!RunPass<NamespaceAnalyzer>(name_resolver))
    return;
  if (!RunPass<ClassAnalyzer>(name_resolver))
    return;
  if (!RunPass<MethodAnalyzer>(name_resolver))
    return;

  Zone zone;
  VariableAnalyzer variable_analyzer(&zone);
  CodeGenerator(this, factory, &variable_analyzer).Run();
  if (HasError())
    return;
  auto const variable_usages = variable_analyzer.Analyze();
  for (auto const method_function : function_map_) {
    auto const function = method_function.second;
    hir::Editor editor(factory, function);
    CfgToSsaConverter(&editor, variable_usages).Run();
  }
}

hir::Function* CompilationSession::FunctionOf(ast::Method* ast_method) {
  auto const method = analysis()->SemanticOf(ast_method)->as<sm::Method>();
  DCHECK(method) << ast_method;
  auto const it = function_map_.find(method);
  return it == function_map_.end() ? nullptr : it->second;
}

void CompilationSession::RegisterFunction(ast::Method* ast_method,
                                          hir::Function* function) {
  auto const method = analysis()->SemanticOf(ast_method)->as<sm::Method>();
  DCHECK(method) << ast_method;
  DCHECK(!function_map_.count(method));
  function_map_.insert(std::make_pair(method, function));
}

}  // namespace compiler
}  // namespace elang
