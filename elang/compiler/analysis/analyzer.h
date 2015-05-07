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

class NameResolver;
class Semantics;

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
class Analyzer : public CompilationSessionUser {
 public:
  NameResolver* name_resolver() const { return name_resolver_; }

 protected:
  explicit Analyzer(NameResolver* resolver);
  virtual ~Analyzer();

  sm::Editor* editor() const { return editor_.get(); }
  sm::Factory* factory() const { return semantics_factory(); }
  NameResolver* resolver() const { return name_resolver_; }
  sm::Factory* semantics_factory() const;

  void FixSemanticOf(ast::Node* node, sm::Semantic* semantic);
  // Shortcut to |NameResolver|.
  sm::Semantic* Resolve(ast::NamedNode* ast_node);
  sm::Type* ResolveTypeReference(ast::Type* reference,
                                 ast::ContainerNode* container);
  void SetSemanticOf(ast::Node* node, sm::Semantic* semantic);

 private:
  std::unique_ptr<sm::Editor> editor_;
  NameResolver* const name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(Analyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_ANALYZER_H_
