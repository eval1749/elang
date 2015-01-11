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
#include "elang/compiler/analyze/analyzer.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/predefined_names.h"

namespace elang {
namespace compiler {

namespace ir {
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

  bool Run();

 private:
  class AnalyzeNode;
  struct ResolveContext;

  void AnalyzeClass(ast::Class* clazz);
  void DidResolve(AnalyzeNode* node);

  void FindInClass(Token* name,
                   ast::Class* clazz,
                   std::unordered_set<ast::NamedNode*>* founds);
  ast::NamedNode* FindResolved(ast::Expression* reference);

  // Returns default base class name for |clazz|, for class it is |Object|,
  // for struct it is |Value|.
  Token* GetDefaultBaseClassName(ast::Class* clazz);

  // Returns access node to default base class name for |clazz|.
  ast::Expression* GetDefaultBaseClassNameAccess(ast::Class* clazz);

  AnalyzeNode* GetOrCreateNode(ast::NamedNode* member);
  ast::NamedNode* GetResolved(ast::Expression* reference);

  Maybe<ast::NamedNode*> Postpone(AnalyzeNode* node, AnalyzeNode* using_node);
  Maybe<ast::NamedNode*> Remember(ast::Expression* reference,
                                  ast::NamedNode* member);

  // Resolve |nth| |base_class_name| of |clazz|.
  Maybe<ir::Class*> ResolveBaseClass(const ResolveContext& context,
                                     ast::Expression* base_class_name,
                                     int nth,
                                     ast::Class* clazz);

  // Resolve default base class of |clazz|.
  Maybe<ir::Class*> ResolveDefaultBaseClass(const ResolveContext& context,
                                            ast::Class* clazz);

  Maybe<ast::NamedNode*> ResolveMemberAccess(const ResolveContext& context,
                                             ast::MemberAccess* reference);
  Maybe<ast::NamedNode*> ResolveNameReference(const ResolveContext& context,
                                              ast::NameReference* reference);
  Maybe<ast::NamedNode*> ResolveReference(const ResolveContext& context,
                                          ast::Expression* reference);

  // ast::Visitor
  void VisitAlias(ast::Alias* alias);
  void VisitClass(ast::Class* clazz);
  void VisitImport(ast::Import* import);
  // Build namespace tree and schedule member to resolve.
  void VisitNamespace(ast::Namespace* ast_Namespace);

  // Cache for mapping reference to resolved entity for alias target and
  // base class list.
  std::unordered_map<ast::Expression*, ast::NamedNode*> reference_cache_;
  std::unordered_map<ast::NamedNode*, AnalyzeNode*> map_;
  std::unordered_set<AnalyzeNode*> unresolved_nodes_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_NAMESPACE_ANALYZER_H_
