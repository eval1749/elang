// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_name_resolver_h)
#define INCLUDE_elang_compiler_name_resolver_h

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/macros.h"
#include "elang/base/types.h"

namespace elang {
namespace hir {
class Class;
class Factory;
class Namespace;
class NamespaceMember;
class Node;
class SimpleName;
}
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
// NameResolver
//
class NameResolver final {
  private: class ScopedResolver;
  friend class ScopedResolver;

  private: hir::Factory* const factory_;
  private: std::vector<ast::NamespaceMember*> pending_members_;
  private: std::unordered_set<ast::NamespaceMember*> pending_set_;
  private: std::unordered_set<ast::NamespaceMember*> running_set_;
  private: std::vector<ast::NamespaceMember*> running_stack_;
  private: std::unordered_set<ast::NamespaceMember*> waiting_set_;
  private: std::unordered_map<ast::Node*, hir::Node*> resolve_map_;
  private: CompilationSession* const session_;

  public: NameResolver(CompilationSession* session, hir::Factory* factory);
  public: ~NameResolver();

  private: void BindAlias(ast::Alias* alias);
  private: Maybe<hir::NamespaceMember*> BindClass(ast::Class* clazz);
  private: void BuildNamespace(hir::Namespace* enclosing_namespace,
                               ast::Namespace* ast_Namespace);
  private: void BuildNamespaceTree(hir::Namespace* enclosing_namespace,
                                   ast::Namespace* ast_Namespace);
  private: Maybe<hir::NamespaceMember*> Resolve(ast::NamespaceMember* member);

  // Resolves left most simple name of |name| in
  // |enclosing_namespace| and |alias_namespace|.
  private: ast::NamespaceMember* ResolveLeftMostName(
      ast::Namespace* enclosing_namespace,
      ast::NamespaceBody* alias_namespace,
      const QualifiedName& name);
  // Resolves |name| in |enclosing_namespace| and |alias_namespace|.
  private: ast::NamespaceMember* ResolveQualifiedName(
      ast::Namespace* enclosing_namespace,
      ast::NamespaceBody* alias_namespace,
      const QualifiedName& name);
  public: bool Run();
  private: void ScheduleClassTree(ast::Class* clazz);
  private: void Schedule(ast::NamespaceMember* member);

  DISALLOW_COPY_AND_ASSIGN(NameResolver);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_name_resolver_h)
