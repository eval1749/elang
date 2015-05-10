// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_ANALYSIS_EDITOR_H_
#define ELANG_COMPILER_ANALYSIS_ANALYSIS_EDITOR_H_

#include "base/macros.h"

namespace elang {
namespace compiler {
class Analysis;
namespace ast {
class Node;
}
namespace sm {
class Semantic;
class Type;
}

class NameResolver;

//////////////////////////////////////////////////////////////////////
//
// AnalysisEditor
//
class AnalysisEditor final {
 public:
  explicit AnalysisEditor(Analysis* analysis);
  ~AnalysisEditor();

  sm::Semantic* SemanticOf(ast::Node* node) const;
  void SetSemanticOf(ast::Node* node, sm::Semantic* semantic);
  sm::Semantic* TrySemanticOf(ast::Node* node) const;

 private:
  Analysis* const analysis_;

  DISALLOW_COPY_AND_ASSIGN(AnalysisEditor);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_ANALYSIS_EDITOR_H_
