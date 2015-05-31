// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/analyzer.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/analysis_editor.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
Analyzer::Analyzer(NameResolver* name_resolver)
    : CompilationSessionUser(name_resolver->session()),
      analysis_editor_(new AnalysisEditor(session()->analysis())),
      editor_(new sm::Editor(session())),
      name_resolver_(name_resolver) {
}

Analyzer::~Analyzer() {
}

sm::Factory* Analyzer::semantic_factory() const {
  return editor_->factory();
}

sm::Type* Analyzer::EnsureType(ast::Type* reference, sm::Semantic* semantic) {
  if (auto const type = semantic->as<sm::Type>())
    return type;
  Error(ErrorCode::AnalyzeTypeNotType, reference);
  auto const type = semantic_factory()->NewUndefinedType(reference->token());
  SetSemanticOf(reference, type);
  return type;
}

sm::Semantic* Analyzer::Resolve(ast::NamedNode* ast_node) {
  return name_resolver_->SemanticOf(ast_node);
}

sm::Type* Analyzer::ResolveTypeReference(ast::Type* reference,
                                         ast::ContainerNode* container) {
  if (auto const semantic = TrySemanticOf(reference))
    return EnsureType(reference, semantic);
  if (auto const array_type = reference->as<ast::ArrayType>()) {
    auto const element_type =
        ResolveTypeReference(array_type->element_type(), container);
    std::vector<int> dimensions(array_type->dimensions().begin(),
                                array_type->dimensions().end());
    auto const value = factory()->NewArrayType(element_type, dimensions);
    SetSemanticOf(reference, value);
    return value;
  }
  if (auto const semantic =
          name_resolver_->ResolveReference(reference, container)) {
    return EnsureType(reference, semantic);
  }

  Error(ErrorCode::AnalyzeTypeNotFound, reference);
  auto const semantic =
      semantic_factory()->NewUndefinedType(reference->token());
  SetSemanticOf(reference, semantic);
  return semantic;
}

void Analyzer::SetSemanticOf(ast::Node* node, sm::Semantic* semantic) {
  analysis_editor_->SetSemanticOf(node, semantic);
}

sm::Semantic* Analyzer::SemanticOf(ast::Node* node) const {
  return analysis()->SemanticOf(node);
}

sm::Semantic* Analyzer::TrySemanticOf(ast::Node* node) const {
  return analysis_editor_->TrySemanticOf(node);
}

}  // namespace compiler
}  // namespace elang
