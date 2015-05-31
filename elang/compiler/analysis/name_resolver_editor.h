// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_EDITOR_H_
#define ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_EDITOR_H_

#include "base/macros.h"

namespace elang {
namespace compiler {
class NameResolver;
namespace ast {
class Alias;
class ContainerNode;
class Import;
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

  sm::Namespace* ImportedNamespaceOf(ast::Import* import) const;
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
