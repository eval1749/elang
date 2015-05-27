// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_NAME_TREE_BUILDER_H_
#define ELANG_COMPILER_ANALYSIS_NAME_TREE_BUILDER_H_

#include "base/macros.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
class AnalysisEditor;
namespace sm {
class Semantic;
}

//////////////////////////////////////////////////////////////////////
//
// NameTreeBuilder
//
class NameTreeBuilder final : public CompilationSessionUser,
                              public ast::Visitor {
 public:
  NameTreeBuilder(CompilationSession* session, AnalysisEditor* eidtor);
  ~NameTreeBuilder();

  // The entry point of |NameTreeBuilder|.
  void Run();

 private:
  sm::Semantic* SemanticOf(ast::Node* node) const;

  // ast::Visitor
  void VisitClassBody(ast::ClassBody* node);
  void VisitConst(ast::Const* node);
  void VisitEnum(ast::Enum* node);
  void VisitField(ast::Field* node);
  void VisitMethod(ast::Method* node);
  void VisitNamespaceBody(ast::NamespaceBody* node);

  AnalysisEditor* const editor_;

  DISALLOW_COPY_AND_ASSIGN(NameTreeBuilder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_NAME_TREE_BUILDER_H_
