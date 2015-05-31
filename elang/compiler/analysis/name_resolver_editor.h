// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_EDITOR_H_
#define ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_EDITOR_H_

#include <unordered_set>

#include "base/macros.h"

namespace elang {
namespace compiler {
class NameResolver;
class Token;
namespace ast {
class Alias;
class ContainerNode;
class Import;
class NamespaceBody;
}
namespace sm {
class Namespace;
class Semantic;
}

//////////////////////////////////////////////////////////////////////
//
// NameResolverEditor
//
class NameResolverEditor final {
 public:
  explicit NameResolverEditor(NameResolver* resolver);
  ~NameResolverEditor();

  NameResolver* resolver() const { return resolver_; }

  void FindWithImports(Token* name,
                       ast::NamespaceBody* ns_body,
                       std::unordered_set<sm::Semantic*>* founds);
  void RegisterAlias(ast::Alias* alias, ast::ContainerNode* resolved);
  void RegisterAlias(ast::Alias* alias, sm::Semantic* resolved);
  void RegisterImport(ast::Import* import, ast::ContainerNode* resolved);
  void RegisterImport(ast::Import* import, sm::Namespace* ns);

 private:
  NameResolver* const resolver_;

  DISALLOW_COPY_AND_ASSIGN(NameResolverEditor);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_EDITOR_H_
