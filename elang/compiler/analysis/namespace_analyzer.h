// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_NAMESPACE_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_NAMESPACE_ANALYZER_H_

#include <unordered_set>
#include <unordered_map>

#include "base/macros.h"
#include "elang/base/maybe.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/predefined_names.h"

namespace elang {
namespace compiler {

namespace sm {
class Factory;
class Class;
}

class CompilationSession;
enum class ErrorCode;
class NameResolver;
class Token;

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
class NamespaceAnalyzer final : public Analyzer,
                                public ZoneOwner,
                                private ast::Visitor {
 public:
  explicit NamespaceAnalyzer(NameResolver* resolver);
  ~NamespaceAnalyzer();

  // The entry point of |NamespaceAnalyzer|.
  void Run();

 private:
  struct ResolveContext;

  void CheckPartialClass(ast::ClassBody* class_body);
  void DidResolve(ast::NamedNode* node);
  void EnsureNamespace(ast::NamespaceBody* namespace_body);
  void FindInClass(Token* name,
                   sm::Class* clazz,
                   std::unordered_set<ast::NamedNode*>* founds);
  ast::NamedNode* FindResolvedReference(ast::Expression* reference);
  sm::Class* GetClass(ast::Class* ast_class);

  // Returns default base class name for |clazz|, for class it is |Object|,
  // for struct it is |Value|.
  Token* GetDefaultBaseClassName(ast::Class* clazz);

  // Returns access node to default base class name for |clazz|.
  ast::Expression* GetDefaultBaseClassNameAccess(ast::Class* clazz);

  ast::NamedNode* GetResolvedReference(ast::Expression* reference);

  bool HasDependency(ast::NamedNode* node) const;
  bool IsResolved(ast::NamedNode* node) const;
  bool IsSystemObject(ast::NamedNode* node) const;
  bool IsVisited(ast::NamedNode* node) const;

  Maybe<ast::NamedNode*> Postpone(ast::NamedNode* node,
                                  ast::NamedNode* using_node);
  Maybe<ast::NamedNode*> Remember(ast::Expression* reference,
                                  ast::NamedNode* member);

  // Resolve |nth| |base_class_name| of |clazz|.
  Maybe<sm::Class*> ResolveBaseClass(const ResolveContext& context,
                                     ast::Expression* base_class_name,
                                     int nth,
                                     ast::Class* clazz);

  // Resolve default base class of |clazz|.
  Maybe<sm::Class*> ResolveDefaultBaseClass(const ResolveContext& context,
                                            ast::Class* clazz);

  Maybe<ast::NamedNode*> ResolveMemberAccess(const ResolveContext& context,
                                             ast::MemberAccess* reference);
  Maybe<ast::NamedNode*> ResolveNameReference(const ResolveContext& context,
                                              ast::NameReference* reference);
  Maybe<ast::NamedNode*> ResolveReference(const ResolveContext& context,
                                          ast::Expression* reference);

  // ast::Visitor
  void VisitAlias(ast::Alias* node);
  void VisitClassBody(ast::ClassBody* node);
  void VisitEnum(ast::Enum* node);
  void VisitImport(ast::Import* node);
  void VisitNamespaceBody(ast::NamespaceBody* node);

  SimpleDirectedGraph<ast::NamedNode*> dependency_graph_;

  // Cache for mapping reference to resolved entity for alias target and
  // base class list.
  std::unordered_map<ast::Expression*, ast::NamedNode*> reference_cache_;
  std::unordered_set<ast::NamedNode*> resolved_nodes_;
  std::unordered_set<ast::NamedNode*> visited_nodes_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_NAMESPACE_ANALYZER_H_
