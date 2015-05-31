// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_NAME_TREE_BUILDER_H_
#define ELANG_COMPILER_ANALYSIS_NAME_TREE_BUILDER_H_

#include <vector>

#include "base/macros.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
class AnalysisEditor;
namespace sm {
class Class;
class Factory;
class Namespace;
class Semantic;
}

//////////////////////////////////////////////////////////////////////
//
// NameTreeBuilder
//
class NameTreeBuilder final : public CompilationSessionUser,
                              public ast::Visitor {
 public:
  NameTreeBuilder(CompilationSession* session, AnalysisEditor* editor);
  ~NameTreeBuilder();

  // The entry point of |NameTreeBuilder|.
  void Run();

 private:
  sm::Factory* factory() const;

  sm::Class* NewClass(ast::ClassBody* node);
  void ProcessNamespaceBody(ast::NamespaceBody* node);
  sm::Semantic* SemanticOf(ast::Node* node) const;

  // ast::Visitor
  void VisitAlias(ast::Alias* alias) final;
  void VisitClassBody(ast::ClassBody* node) final;
  void VisitConst(ast::Const* node) final;
  void VisitEnum(ast::Enum* node) final;
  void VisitField(ast::Field* node) final;
  void VisitMethod(ast::Method* node) final;
  void VisitNamespaceBody(ast::NamespaceBody* node) final;

  std::vector<ast::Alias*> aliases_;
  AnalysisEditor* const editor_;

  DISALLOW_COPY_AND_ASSIGN(NameTreeBuilder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_NAME_TREE_BUILDER_H_
