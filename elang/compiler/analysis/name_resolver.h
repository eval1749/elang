// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_H_
#define ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_H_

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "base/macros.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
namespace ast {
class Call;
class Class;
class Expression;
class ContainerNode;
class NamedNode;
}
namespace sm {
class Factory;
class Method;
class Semantic;
class Type;
}

class Analysis;
class NameResolverEditor;
enum class PredefinedName;
class Token;

//////////////////////////////////////////////////////////////////////
//
// NameResolver
// Keeps analysis results from |NamespaceAnalyzer| for mapping name reference
// to |ast::NamedNode|.
//
class NameResolver final : public CompilationSessionUser {
 public:
  explicit NameResolver(CompilationSession* session);
  ~NameResolver();

  sm::Factory* factory() const;

  sm::Semantic* ResolveReference(ast::Expression* expression,
                                 ast::ContainerNode* container);
  sm::Semantic* SemanticOf(ast::NamedNode* ast_node) const;

 private:
  friend class NameResolverEditor;
  class ReferenceResolver;

  // Returns |ContainerNode| associated to |Alias| or |Import| |node|.
  ast::ContainerNode* GetUsingReference(ast::NamedNode* node);

  // Mapping from |Alias| or |Import| to |ContainerNode|.
  std::unordered_map<ast::NamedNode*, ast::ContainerNode*> using_map_;

  DISALLOW_COPY_AND_ASSIGN(NameResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_NAME_RESOLVER_H_
