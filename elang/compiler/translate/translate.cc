// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/compilation_session.h"

#include "elang/compiler/analysis/class_analyzer.h"
#include "elang/compiler/analysis/method_analyzer.h"
#include "elang/compiler/analysis/namespace_analyzer.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/translate/translator.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/nodes.h"

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
                                 ir::Factory* factory) {
  if (!errors().empty())
    return false;
  if (!Run<NamespaceAnalyzer>(name_resolver))
    return false;
  if (!Run<ClassAnalyzer>(name_resolver))
    return false;
  if (!Run<MethodAnalyzer>(name_resolver))
    return false;

  Translator translator(this, factory);
  if (!translator.Run())
    return false;
  return errors().empty();
}

ir::Function* CompilationSession::IrFunctionOf(ast::Method* method) {
  auto const it = ir_function_map_.find(method);
  return it == ir_function_map_.end() ? nullptr : it->second;
}

void CompilationSession::RegisterFunction(ast::Method* method,
                                          ir::Function* function) {
  DCHECK(!ir_function_map_.count(method));
  ir_function_map_[method] = function;
}

}  // namespace compiler
}  // namespace elang
