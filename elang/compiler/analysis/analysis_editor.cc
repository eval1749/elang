// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/analysis_editor.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// AnalysisEditor
//
AnalysisEditor::AnalysisEditor(Analysis* analysis) : analysis_(analysis) {
}

AnalysisEditor::~AnalysisEditor() {
}

void AnalysisEditor::SetSemanticOf(ast::Node* node, sm::Semantic* semantic) {
  DCHECK(node);
  DCHECK(semantic);
  auto const it = analysis_->semantic_map_.find(node);
  DCHECK(it == analysis_->semantic_map_.end())
      << *node << " old:" << *it->second << " new:" << *semantic;
  analysis_->semantic_map_[node] = semantic;
}

sm::Semantic* AnalysisEditor::SemanticOf(ast::Node* node) const {
  auto const semantic = TrySemanticOf(node);
  DCHECK(semantic) << "No semantic for " << *node;
  return semantic;
}

sm::Semantic* AnalysisEditor::TrySemanticOf(ast::Node* node) const {
  auto const it = analysis_->semantic_map_.find(node);
  return it == analysis_->semantic_map_.end() ? nullptr : it->second;
}

}  // namespace compiler
}  // namespace elang
