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

enum class PredefinedName;
class Semantics;
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

  // Registering functions.
  void DidResolve(ast::NamedNode* ast_node, sm::Semantic* node);
  void DidResolveUsing(ast::NamedNode* ast_node, ast::ContainerNode* container);

  // Retrieving functions.
  sm::Type* PredefinedTypeOf(PredefinedName name);
  sm::Semantic* Resolve(ast::NamedNode* ast_node) const;
  // Resolve to |sm::Type| named |name| for |token|.
  sm::Type* ResolvePredefinedType(Token* token, PredefinedName name);
  ast::NamedNode* ResolveReference(ast::Expression* expression,
                                   ast::ContainerNode* container);

 private:
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
