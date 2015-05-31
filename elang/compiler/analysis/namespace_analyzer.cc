// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/namespace_analyzer.h"

#include "elang/compiler/analysis/analysis_editor.h"
#include "elang/compiler/analysis/class_tree_builder.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/analysis/name_tree_builder.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/editor.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
NamespaceAnalyzer::NamespaceAnalyzer(NameResolver* resolver)
    : resolver_(resolver) {
}

NamespaceAnalyzer::~NamespaceAnalyzer() {
}

CompilationSession* NamespaceAnalyzer::session() const {
  return resolver_->session();
}

// The entry point of |NamespaceAnalyzer|.
void NamespaceAnalyzer::Run() {
  AnalysisEditor analysis_editor(session()->analysis());
  NameTreeBuilder(session(), &analysis_editor).Run();
  if (session()->HasError())
    return;
  sm::Editor semantic_editor(session());
  ClassTreeBuilder(resolver_, &semantic_editor).Run();
}

}  // namespace compiler
}  // namespace elang
