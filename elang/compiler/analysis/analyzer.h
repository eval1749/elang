// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_ANALYZER_H_

#include <unordered_map>

#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {

namespace sm {
class Editor;
class Factory;
class Semantic;
class Type;
}

class AnalysisEditor;
class NameResolver;

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
class Analyzer : public CompilationSessionUser {
 public:
  sm::Editor* editor() const { return editor_.get(); }
  sm::Factory* factory() const { return semantic_factory(); }
  NameResolver* name_resolver() const { return name_resolver_; }

  sm::Type* ResolveTypeReference(ast::Type* reference,
                                 ast::ContainerNode* container);
  sm::Semantic* SemanticOf(ast::Node* node) const;
  void SetSemanticOf(ast::Node* node, sm::Semantic* semantic);
  sm::Semantic* TrySemanticOf(ast::Node* node) const;

 protected:
  explicit Analyzer(NameResolver* resolver);
  virtual ~Analyzer();

  NameResolver* resolver() const { return name_resolver_; }
  sm::Factory* semantic_factory() const;

  // Shortcut to |NameResolver|.
  sm::Semantic* Resolve(ast::NamedNode* ast_node);

 private:
  sm::Type* EnsureType(ast::Type* reference, sm::Semantic* semantic);

  std::unique_ptr<AnalysisEditor> analysis_editor_;
  std::unique_ptr<sm::Editor> editor_;
  NameResolver* const name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(Analyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_ANALYZER_H_
