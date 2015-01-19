// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "base/macros.h"

namespace elang {
namespace compiler {
namespace ast {
class Call;
class Expression;
class ContainerNode;
class NamedNode;
}
namespace ir {
class Node;
class Factory;
class Method;
class Type;
}

class CompilationSession;
enum class PredefinedName;
class Semantics;
class Token;

//////////////////////////////////////////////////////////////////////
//
// NameResolver
// Keeps analysis results from |NamespaceAnalyzer| for mapping name reference
// to |ast::NamedNode|.
//
class NameResolver final {
 public:
  explicit NameResolver(CompilationSession* session);
  ~NameResolver();

  ir::Factory* factory() const { return factory_.get(); }
  Semantics* semantics() const;
  CompilationSession* session() const { return session_; }

  // Registering functions.
  void DidResolve(ast::NamedNode* ast_node, ir::Node* node);
  void DidResolveUsing(ast::NamedNode* ast_node, ast::ContainerNode* container);

  // Retrieving functions.
  ir::Type* GetPredefinedType(PredefinedName name);
  ir::Node* Resolve(ast::NamedNode* ast_node) const;
  // Resolve to |ir::Type| named |name| for |token|.
  ir::Type* ResolvePredefinedType(Token* token, PredefinedName name);
  ast::NamedNode* ResolveReference(ast::Expression* expression,
                                   ast::ContainerNode* container);

 private:
  class ReferenceResolver;

  // Returns |ContainerNode| associated to |Alias| or |Import| |node|.
  ast::ContainerNode* GetUsingReference(ast::NamedNode* node);

  const std::unique_ptr<ir::Factory> factory_;
  CompilationSession* const session_;
  // Mapping from |Alias| or |Import| to |ContainerNode|.
  std::unordered_map<ast::NamedNode*, ast::ContainerNode*> using_map_;

  DISALLOW_COPY_AND_ASSIGN(NameResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
