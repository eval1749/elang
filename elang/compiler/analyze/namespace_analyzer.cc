// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <deque>
#include <vector>

#include "elang/compiler/analyze/namespace_analyzer.h"

#include "base/logging.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_unordered_set.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer::ResolveContext
//
struct NamespaceAnalyzer::ResolveContext {
  // A |container| for looking up name. Except for |ast::Alias| node, it is
  // enclosing container. For |ast::Alias| it is enclosing container of
  // enclosing container.
  ast::ContainerNode* container;
  // This resolve request is part of member access.
  ast::MemberAccess* member_access;
  // Requester of resolving |node|
  ast::NamedNode* node;

  ResolveContext(ast::NamedNode* node, ast::ContainerNode* container);
};

// Resolve name in |namespace|.
NamespaceAnalyzer::ResolveContext::ResolveContext(ast::NamedNode* node,
                                                  ast::ContainerNode* container)
    : member_access(nullptr), container(container), node(node) {
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
NamespaceAnalyzer::NamespaceAnalyzer(NameResolver* resolver)
    : Analyzer(resolver) {
}

NamespaceAnalyzer::~NamespaceAnalyzer() {
}

void NamespaceAnalyzer::DidResolve(ast::NamedNode* node) {
  DCHECK(!resolved_nodes_.count(node));
  resolved_nodes_.insert(node);
  std::vector<ast::NamedNode*> users(dependency_graph_.GetInEdges(node));
  for (auto const user : users) {
    dependency_graph_.RemoveEdge(user, node);
    if (HasDependency(user))
      continue;
    user->Accept(this);
  }
}

// Find |name| in class tree rooted by |clazz|.
void NamespaceAnalyzer::FindInClass(
    Token* name,
    ast::Class* ast_class,
    std::unordered_set<ast::NamedNode*>* founds) {
  DCHECK(IsResolved(ast_class));
  if (auto const present = ast_class->FindMember(name)) {
    founds->insert(present);
    return;
  }
  for (auto const base_class_name : ast_class->base_class_names()) {
    auto const base_class =
        GetResolvedReference(base_class_name)->as<ast::Class>();
    FindInClass(name, base_class, founds);
  }
}

ast::NamedNode* NamespaceAnalyzer::FindResolvedReference(
    ast::Expression* reference) {
  auto const it = reference_cache_.find(reference);
  return it == reference_cache_.end() ? nullptr : it->second;
}

Token* NamespaceAnalyzer::GetDefaultBaseClassName(ast::Class* clazz) {
  return session()->NewToken(
      clazz->name()->location(),
      session()->name_for(clazz->is_class() ? PredefinedName::Object
                                            : PredefinedName::Value));
}

ast::Expression* NamespaceAnalyzer::GetDefaultBaseClassNameAccess(
    ast::Class* clazz) {
  return session()->ast_factory()->NewMemberAccess(
      clazz->name(), {session()->ast_factory()->NewNameReference(
                          session()->system_namespace()->name()),
                      session()->ast_factory()->NewNameReference(
                          GetDefaultBaseClassName(clazz))});
}

ast::NamedNode* NamespaceAnalyzer::GetResolvedReference(
    ast::Expression* reference) {
  auto const present = FindResolvedReference(reference);
  DCHECK(present);
  return present;
}

bool NamespaceAnalyzer::HasDependency(ast::NamedNode* node) const {
  return dependency_graph_.HasOutEdge(node);
}

bool NamespaceAnalyzer::IsResolved(ast::NamedNode* node) const {
  if (resolver()->Resolve(node))
    return true;
  return !!resolved_nodes_.count(node);
}

bool NamespaceAnalyzer::IsSystemObject(ast::NamedNode* node) const {
  auto const ast_class = node->as<ast::Class>();
  if (!ast_class)
    return false;
  if (ast_class->name()->simple_name() !=
      session()->name_for(PredefinedName::Object)) {
    return false;
  }
  for (auto runner = ast_class->parent(); runner; runner = runner->parent()) {
    if (runner == session()->system_namespace())
      return true;
  }
  return false;
}

bool NamespaceAnalyzer::IsVisited(ast::NamedNode* node) const {
  return !!visited_nodes_.count(node);
}

Maybe<ast::NamedNode*> NamespaceAnalyzer::Postpone(ast::NamedNode* node,
                                                   ast::NamedNode* using_node) {
  dependency_graph_.AddEdge(node, using_node);
  return Maybe<ast::NamedNode*>();
}

Maybe<ast::NamedNode*> NamespaceAnalyzer::Remember(ast::Expression* reference,
                                                   ast::NamedNode* member) {
  DCHECK(!reference_cache_.count(reference));
  reference_cache_[reference] = member;
  return Maybe<ast::NamedNode*>(member);
}

Maybe<ir::Class*> NamespaceAnalyzer::ResolveBaseClass(
    const ResolveContext& context,
    ast::Expression* base_class_name,
    int nth,
    ast::Class* clazz) {
  DCHECK_GE(nth, 1);
  auto const result = ResolveReference(context, base_class_name);
  if (!result.has_value)
    return Maybe<ir::Class*>();
  if (!result.value)
    return Maybe<ir::Class*>(nullptr);

  auto const base_class = result.value->as<ast::Class>();
  if (!base_class) {
    Error(ErrorCode::NameResolutionNameNeitherClassNorInterface,
          base_class_name);
    return Maybe<ir::Class*>(nullptr);
  }

  if (clazz->is_class()) {
    if (nth == 1) {
      if (base_class->is_struct()) {
        Error(ErrorCode::NameResolutionNameNeitherClassNorInterface,
              base_class_name);
        return Maybe<ir::Class*>(nullptr);
      }
    } else if (nth >= 2 && !base_class->is_interface()) {
      Error(ErrorCode::NameResolutionNameNotInterface, base_class_name);
      return Maybe<ir::Class*>(nullptr);
    }
  } else if (!base_class->is_interface()) {
    // interface and struct have interface only.
    Error(ErrorCode::NameResolutionNameNotInterface, base_class_name);
    return Maybe<ir::Class*>(nullptr);
  }

  // TODO(eval1749) Check |base_class| isn't |final|.
  // TODO(eval1749) We should check accessibility of |base_class|.
  if (base_class == clazz->parent() ||
      clazz->parent()->IsDescendantOf(base_class)) {
    Error(ErrorCode::NameResolutionClassContaining, base_class_name, clazz);
    return Maybe<ir::Class*>(nullptr);
  }

  if (!IsResolved(base_class)) {
    Postpone(clazz, base_class);
    return Maybe<ir::Class*>();
  }

  auto const data = Resolve(base_class);
  if (!data) {
    Error(ErrorCode::NameResolutionClassNotResolved, base_class_name);
    return Maybe<ir::Class*>(nullptr);
  }
  if (!data->is<ir::Class>()) {
    Error(ErrorCode::NameResolutionClassNotClass, base_class_name);
    return Maybe<ir::Class*>(nullptr);
  }
  return Maybe<ir::Class*>(data->as<ir::Class>());
}

Maybe<ir::Class*> NamespaceAnalyzer::ResolveDefaultBaseClass(
    const ResolveContext& context,
    ast::Class* clazz) {
  auto const default_base_class_name = GetDefaultBaseClassNameAccess(clazz);
  auto const result = ResolveReference(context, default_base_class_name);
  if (!result.has_value)
    return Maybe<ir::Class*>();
  if (!result.value)
    return Maybe<ir::Class*>(nullptr);
  auto const default_base_class = result.value;
  if (!IsResolved(default_base_class)) {
    Postpone(clazz, default_base_class);
    return Maybe<ir::Class*>();
  }
  auto const resolved = Resolve(default_base_class);
  if (!resolved) {
    Error(ErrorCode::PredefinedNamesNameNotFound, default_base_class);
    return Maybe<ir::Class*>(nullptr);
  }
  if (auto const base_class = resolved->as<ir::Class>())
    return Maybe<ir::Class*>(base_class);
  Error(ErrorCode::PredefinedNamesNameNotClass, default_base_class);
  return Maybe<ir::Class*>(nullptr);
}

Maybe<ast::NamedNode*> NamespaceAnalyzer::ResolveMemberAccess(
    const ResolveContext& start_context,
    ast::MemberAccess* reference) {
  ResolveContext context(start_context);
  context.member_access = reference;
  auto resolved = static_cast<ast::NamedNode*>(nullptr);
  for (auto const component : reference->components()) {
    if (resolved) {
      auto const container = resolved->as<ast::ContainerNode>();
      if (!container) {
        Error(ErrorCode::NameResolutionNameNeitherNamespaceNorType, component,
              reference);
        return Remember(reference, nullptr);
      }
      context.container = container;
    }

    auto const result = ResolveReference(context, component);
    if (!result.has_value)
      return result;
    if (!result.value)
      return Remember(reference, nullptr);
    resolved = result.value;
  }
  DCHECK(resolved);
  return Remember(reference, resolved);
}

Maybe<ast::NamedNode*> NamespaceAnalyzer::ResolveNameReference(
    const ResolveContext& context,
    ast::NameReference* reference) {
  auto const name = reference->name();
  auto container = context.container;
  while (container) {
    std::unordered_set<ast::NamedNode*> founds;

    if (auto const ast_class = container->as<ast::Class>()) {
      if (auto const present = ast_class->FindMember(name)) {
        founds.insert(present);
      } else if (!IsResolved(ast_class)) {
        return Postpone(context.node, ast_class);
      } else {
        // |name| isn't member of |ast_class|. We try to search |name| in
        // base class tree.
        for (auto const base_class_name : ast_class->base_class_names()) {
          auto const resolved = FindResolvedReference(base_class_name);
          if (!resolved)
            return Remember(reference, nullptr);
          auto const base_class = resolved->as<ast::Class>();
          FindInClass(name, base_class, &founds);
        }
      }

    } else if (auto const ns = container->as<ast::Namespace>()) {
      if (auto const present = ns->FindMember(name))
        founds.insert(present);

    } else if (auto const ns_body = container->as<ast::NamespaceBody>()) {
      // Find in namespace
      if (auto const present = ns_body->owner()->FindMember(name)) {
        DCHECK(!present->is<ast::Alias>());
        founds.insert(present);
      }

      // Find alias
      if (auto const alias = ns_body->FindAlias(name)) {
        if (!IsResolved(alias))
          return Postpone(context.node, alias);
        auto const resolved = FindResolvedReference(alias->reference());
        if (!resolved)
          return Remember(reference, nullptr);
        DCHECK(!resolved->is<ast::Alias>());
        founds.insert(resolved);
      }

      if (!ns_body->FindMember(name)) {
        // When |name| isn't defined in namespace body, looking in imported
        // namespaces.
        for (auto const pair : ns_body->imports()) {
          auto const import = pair.second;
          if (!IsResolved(import))
            return Postpone(context.node, import);
          auto const imported = GetResolvedReference(import->reference());
          auto const imported_ns = imported->as<ast::Namespace>();
          if (!imported_ns)
            continue;
          auto const present = imported_ns->FindMember(name);
          if (!present || present->is<ast::Namespace>())
            continue;
          // Import directive doesn't import nested namespace.
          founds.insert(present);
        }
      }
    } else {
      NOTREACHED();
    }

    if (founds.size() == 1u) {
      auto const found = *founds.begin();
      DCHECK(!found->is<ast::Alias>());
      return Remember(reference, found);
    }

    if (founds.size() >= 2u) {
      Error(ErrorCode::NameResolutionNameAmbiguous, reference);
      return Remember(reference, nullptr);
    }

    // Current name space doesn't contain |name|. Ge enclosing name space.
    container = container->parent();
  }
  if (context.member_access)
    Error(ErrorCode::NameResolutionNameNotResolved, reference);
  else
    Error(ErrorCode::NameResolutionNameNotFound, reference);
  return Remember(reference, nullptr);
}

Maybe<ast::NamedNode*> NamespaceAnalyzer::ResolveReference(
    const ResolveContext& context,
    ast::Expression* reference) {
  if (auto const resolved = FindResolvedReference(reference))
    return Maybe<ast::NamedNode*>(resolved);
  if (auto const name_reference = reference->as<ast::NameReference>())
    return ResolveNameReference(context, name_reference);
  if (auto const member_access = reference->as<ast::MemberAccess>())
    return ResolveMemberAccess(context, member_access);
#if 0
  // TODO(eval1749) Support |ConstructedType| in |NamespaceAnalyzer|.
  if (auto const cons_type = reference->as<ast::ConstructedType>())
    return ResolveConstructedType(context, cons_type);
#endif
  NOTREACHED();
  return Maybe<ast::NamedNode*>(nullptr);
}

// The entry point of |NamespaceAnalyzer|.
bool NamespaceAnalyzer::Run() {
  VisitNamespaceBody(session()->root_node());
  if (!session()->errors().empty())
    return false;
  std::unordered_set<ast::NamedNode*> unresolved_nodes;
  for (auto const node : visited_nodes_) {
    if (IsResolved(node))
      continue;
    unresolved_nodes.insert(node);
  }
  if (unresolved_nodes.empty())
    return true;
  for (auto const node : unresolved_nodes) {
    Error(ErrorCode::NameResolutionNameCycle, node,
          dependency_graph_.GetInEdges(node).front());
  }
  return false;
}

// ast::Visitor

// Reference of |ast::Alias| are resolved in enclosing container of
// enclosing container of |ast::Alias|, e.g. looking into namespace N1 in
// below example:
//
//  namespace N1 {
//    namespace N2 {
//      using R1 = A;
//      class A {}
//      class B : R1 {}  // base_class_of(B) == N1.A
//    }
//    class A {}
//  }
void NamespaceAnalyzer::VisitAlias(ast::Alias* alias) {
  if (!IsVisited(alias))
    visited_nodes_.insert(alias);
  if (IsResolved(alias))
    return;
  ResolveContext context(alias, alias->parent()->parent());
  auto const result = ResolveReference(context, alias->reference());
  if (!result.has_value)
    return;
  if (result.value && !result.value->as<ast::ContainerNode>()) {
    Error(ErrorCode::NameResolutionAliasNeitherNamespaceNorType,
          alias->reference());
  }
  DidResolve(alias);
}

void NamespaceAnalyzer::VisitClass(ast::Class* ast_class) {
  if (IsResolved(ast_class))
    return;

  if (!IsVisited(ast_class)) {
    visited_nodes_.insert(ast_class);
    if (auto const enclosing_class = ast_class->parent()->as<ast::Class>()) {
      if (!IsResolved(enclosing_class))
        Postpone(ast_class, enclosing_class);
    }
    ast_class->AcceptForMembers(this);
  }

  // Resolve direct base classes
  auto are_direct_base_classes_valid = true;
  auto nth = 0;
  ResolveContext context(ast_class, ast_class->parent());
  std::vector<ir::Class*> direct_base_classes;
  for (auto const base_class_name : ast_class->base_class_names()) {
    ++nth;
    auto const result =
        ResolveBaseClass(context, base_class_name, nth, ast_class);
    if (!result.has_value)
      continue;
    if (!result.value) {
      are_direct_base_classes_valid = false;
      continue;
    }
    direct_base_classes.push_back(result.value);
  }

  if (!are_direct_base_classes_valid) {
    DidResolve(ast_class);
    return;
  }

  if (HasDependency(ast_class))
    return;

  if (IsSystemObject(ast_class)) {
    if (!direct_base_classes.empty() &&
        direct_base_classes.front()->is_class()) {
      Error(ErrorCode::NameResolutionSystemObjectHasBaseClass, ast_class);
    }
  }
  if (!ast_class->is_interface() &&
      (direct_base_classes.empty() ||
       !direct_base_classes.front()->is_class())) {
    auto const result = ResolveDefaultBaseClass(context, ast_class);
    if (!result.has_value)
      return;
    if (!result.value) {
      DidResolve(ast_class);
      return;
    }
    direct_base_classes.insert(direct_base_classes.begin(), result.value);
  }

  resolver()->DidResolve(ast_class,
                         factory()->NewClass(ast_class, direct_base_classes));
  DidResolve(ast_class);
}

void NamespaceAnalyzer::VisitImport(ast::Import* import) {
  if (!IsVisited(import))
    visited_nodes_.insert(import);
  if (IsResolved(import))
    return;
  ResolveContext context(import, import->parent()->parent());
  auto const result = ResolveReference(context, import->reference());
  if (!result.has_value)
    return;
  DidResolve(import);
  if (!result.value || result.value->is<ast::Namespace>() ||
      result.value->is<ast::Class>()) {
    return;
  }
  Error(ErrorCode::NameResolutionImportNeitherNamespaceNorType,
        import->reference());
}

// Builds namespace tree and schedule members to resolve.
void NamespaceAnalyzer::VisitNamespaceBody(ast::NamespaceBody* ns_body) {
  ns_body->AcceptForMembers(this);
}

}  // namespace compiler
}  // namespace elang
