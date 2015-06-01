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
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/translate/translator.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/nodes.h"

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
                                 ir::Factory* factory) {
  if (HasError())
    return;
  if (!RunPass<NamespaceAnalyzer>(name_resolver))
    return;
  if (!RunPass<ClassAnalyzer>(name_resolver))
    return;
  if (!RunPass<MethodAnalyzer>(name_resolver))
    return;
  Translator(this, factory).Run();
}

ir::Function* CompilationSession::IrFunctionOf(ast::Method* ast_method) {
  auto const method = analysis()->SemanticOf(ast_method)->as<sm::Method>();
  DCHECK(method) << ast_method;
  auto const it = ir_function_map_.find(method);
  return it == ir_function_map_.end() ? nullptr : it->second;
}

void CompilationSession::RegisterFunction(ast::Method* ast_method,
                                          ir::Function* function) {
  auto const method = analysis()->SemanticOf(ast_method)->as<sm::Method>();
  DCHECK(method) << ast_method;
  DCHECK(!ir_function_map_.count(method));
  ir_function_map_.insert(std::make_pair(method, function));
}

}  // namespace compiler
}  // namespace elang
