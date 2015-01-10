// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_NAMESPACE_ANALYZER_H_
#define ELANG_COMPILER_ANALYZE_NAMESPACE_ANALYZER_H_

#include <unordered_set>
#include <unordered_map>

#include "base/macros.h"
#include "elang/base/maybe.h"
#include "elang/base/zone_owner.h"

namespace elang {
namespace compiler {
class CompilationSession;
class NameResolver;
class Token;

namespace ast {
class Node;
class Alias;
class Class;
class ConstructedType;
class Expression;
class Import;
class MemberAccess;
class Namespace;
class NamespaceBody;
class NamespaceMember;
class NameReference;
}  // namespace ast

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
class NamespaceAnalyzer final : public ZoneOwner {
 public:
  NamespaceAnalyzer(CompilationSession* session, NameResolver* resolver);
  ~NamespaceAnalyzer();

  bool Run();

 private:
  class AnalyzeNode;
  struct ResolveContext;

  void AnalyzeAlias(ast::Alias* alias);
  void AnalyzeClass(ast::Class* clazz);
  void AnalyzeImport(ast::Import* import);
  // Build namespace tree and schedule member to resolve.
  void AnalyzeNamespace(ast::Namespace* ast_Namespace);
  void AnalyzeNamespaceMember(ast::NamespaceMember* member);

  void DidResolve(AnalyzeNode* node);
  std::unordered_set<ast::NamespaceMember*> FindInClass(Token* name,
                                                        ast::Class* clazz);
  ast::NamespaceMember* FindResolved(ast::Expression* reference);
  AnalyzeNode* GetOrCreateNode(ast::NamespaceMember* member);
  ast::NamespaceMember* GetResolved(ast::Expression* reference);

  Maybe<ast::NamespaceMember*> Postpone(AnalyzeNode* node,
                                        AnalyzeNode* using_node);
  Maybe<ast::NamespaceMember*> Remember(ast::Expression* reference,
                                        ast::NamespaceMember* member);

  Maybe<ast::NamespaceMember*> ResolveMemberAccess(
      const ResolveContext& context,
      ast::MemberAccess* reference);
  Maybe<ast::NamespaceMember*> ResolveNameReference(
      const ResolveContext& context,
      ast::NameReference* reference);
  Maybe<ast::NamespaceMember*> ResolveReference(const ResolveContext& context,
                                                ast::Expression* reference);
  // Cache for mapping reference to resolved entity for alias target and
  // base class list.
  std::unordered_map<ast::Expression*, ast::NamespaceMember*> reference_cache_;
  std::unordered_map<ast::NamespaceMember*, AnalyzeNode*> map_;
  NameResolver* const resolver_;
  CompilationSession* const session_;
  std::unordered_set<AnalyzeNode*> unresolved_nodes_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_NAMESPACE_ANALYZER_H_
