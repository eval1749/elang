// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_NAMESPACE_ANALYZER_H_
#define ELANG_COMPILER_ANALYZE_NAMESPACE_ANALYZER_H_

#include <unordered_set>
#include <vector>

#include "base/macros.h"
#include "elang/base/types.h"

namespace elang {
namespace compiler {
class CompilationSession;
class QualifiedName;
class Token;

namespace ast {
class Node;
class Alias;
class Class;
class Namespace;
class NamespaceBody;
class NamespaceMember;
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
class NamespaceAnalyzer final {
 public:
  explicit NamespaceAnalyzer(CompilationSession* session);
  ~NamespaceAnalyzer();

  bool Run();

 private:
  class ScopedResolver;
  friend class ScopedResolver;

  void BindAlias(ast::Alias* alias);
  Maybe<ast::NamespaceMember*> FixClass(ast::Class* clazz);
  void BindMembers(ast::Namespace* ast_Namespace);
  Maybe<ast::NamespaceMember*> Resolve(ast::NamespaceMember* member);

  // Resolves left most simple name of |name| in
  // |enclosing_namespace| and |alias_namespace|.
  ast::NamespaceMember* ResolveLeftMostName(ast::Namespace* enclosing_namespace,
                                            ast::NamespaceBody* alias_namespace,
                                            const QualifiedName& name);
  // Resolves |name| in |enclosing_namespace| and |alias_namespace|.
  ast::NamespaceMember* ResolveQualifiedName(
      ast::Namespace* enclosing_namespace,
      ast::NamespaceBody* alias_namespace,
      const QualifiedName& name);
  void ScheduleClassTree(ast::Class* clazz);
  void Schedule(ast::NamespaceMember* member);

  std::vector<ast::NamespaceMember*> pending_members_;
  std::unordered_set<ast::NamespaceMember*> pending_set_;
  std::unordered_set<ast::NamespaceMember*> running_set_;
  std::vector<ast::NamespaceMember*> running_stack_;
  std::unordered_set<ast::NamespaceMember*> waiting_set_;
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_NAMESPACE_ANALYZER_H_
