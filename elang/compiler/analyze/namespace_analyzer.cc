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

///////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer::AnalyzeNode
// Represents a dependency graph node.
//
class NamespaceAnalyzer::AnalyzeNode final : public ZoneAllocated {
 public:
  AnalyzeNode(Zone* zone, ast::NamedNode* member);

  bool is_resolved() const { return is_resolved_; }
  ast::NamedNode* member() const { return member_; }
  const ZoneUnorderedSet<AnalyzeNode*> uses() const { return uses_; }
  const ZoneUnorderedSet<AnalyzeNode*> users() const { return users_; }

  void DidResolve();
  ast::Node* GetFirstUser() const;
  void RemoveUse(AnalyzeNode* node);
  void Use(AnalyzeNode* node);

 private:
  ~AnalyzeNode() = default;

  bool is_resolved_;
  ast::NamedNode* const member_;
  ZoneUnorderedSet<AnalyzeNode*> uses_;
  ZoneUnorderedSet<AnalyzeNode*> users_;

  DISALLOW_COPY_AND_ASSIGN(AnalyzeNode);
};

NamespaceAnalyzer::AnalyzeNode::AnalyzeNode(Zone* zone, ast::NamedNode* member)
    : is_resolved_(member->is<ast::Namespace>()),
      member_(member),
      uses_(zone),
      users_(zone) {
}

void NamespaceAnalyzer::AnalyzeNode::DidResolve() {
  DCHECK(!is_resolved_);
  is_resolved_ = true;
}

ast::Node* NamespaceAnalyzer::AnalyzeNode::GetFirstUser() const {
  auto node = static_cast<AnalyzeNode*>(nullptr);
  for (auto const runner : users_) {
    if (!node ||
        node->member()->name()->location().start_offset() >
            runner->member()->name()->location().start_offset()) {
      node = runner;
    }
  }
  return node ? node->member() : nullptr;
}

void NamespaceAnalyzer::AnalyzeNode::RemoveUse(AnalyzeNode* node) {
  uses_.erase(node);
  node->users_.erase(this);
}

void NamespaceAnalyzer::AnalyzeNode::Use(AnalyzeNode* node) {
  uses_.insert(node);
  node->users_.insert(this);
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer::ResolveContext
//
struct NamespaceAnalyzer::ResolveContext {
  ast::ContainerNode* container;
  ast::MemberAccess* member_access;
  AnalyzeNode* node;

  ResolveContext(AnalyzeNode* node, ast::ContainerNode* container);
};

// Resolve name in |namespace|.
NamespaceAnalyzer::ResolveContext::ResolveContext(AnalyzeNode* node,
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

void NamespaceAnalyzer::AnalyzeClass(ast::Class* ast_class) {
  auto const class_node = GetOrCreateNode(ast_class);
  if (class_node->is_resolved())
    return;

  if (Resolve(ast_class)) {
    DidResolve(class_node);
    return;
  }

  auto has_value = true;

  // Check enclosing class
  if (auto const enclosing_class = ast_class->parent()->as<ast::Class>()) {
    auto const enclosing_class_node = GetOrCreateNode(enclosing_class);
    if (!enclosing_class_node->is_resolved()) {
      Postpone(class_node, enclosing_class_node);
      has_value = false;
    }
  }

  // Resolve direct base classes
  auto are_direct_base_classes_valid = true;
  auto nth = 0;
  ResolveContext context(class_node, ast_class->parent());
  std::vector<ir::Class*> direct_base_classes;
  for (auto const base_class_name : ast_class->base_class_names()) {
    ++nth;
    auto const result =
        ResolveBaseClass(context, base_class_name, nth, ast_class);
    if (!result.has_value) {
      has_value = false;
      continue;
    }
    if (!result.value) {
      are_direct_base_classes_valid = false;
      continue;
    }
    direct_base_classes.push_back(result.value);
  }

  if (!are_direct_base_classes_valid) {
    DidResolve(class_node);
    return;
  }

  if (!has_value)
    return;

  if (!ast_class->is_interface() &&
      (direct_base_classes.empty() ||
       !direct_base_classes.front()->is_class())) {
    auto const result = ResolveDefaultBaseClass(context, ast_class);
    if (!result.has_value)
      return;
    if (!result.value) {
      DidResolve(class_node);
      return;
    }
    direct_base_classes.insert(direct_base_classes.begin(), result.value);
  }

  resolver()->DidResolve(ast_class,
                         factory()->NewClass(ast_class, direct_base_classes));
  DidResolve(class_node);
}

void NamespaceAnalyzer::DidResolve(AnalyzeNode* node) {
  node->DidResolve();
  unresolved_nodes_.erase(node);
  std::vector<AnalyzeNode*> users;
  for (auto const user : node->users())
    users.push_back(user);
  for (auto const user : users) {
    user->RemoveUse(node);
    if (!user->uses().empty())
      continue;
    user->member()->Accept(this);
  }
}

// Find |name| in class tree rooted by |clazz|.
void NamespaceAnalyzer::FindInClass(
    Token* name,
    ast::Class* ast_class,
    std::unordered_set<ast::NamedNode*>* founds) {
  DCHECK(GetOrCreateNode(ast_class)->is_resolved());
  if (auto const present = ast_class->FindMember(name)) {
    founds->insert(present);
    return;
  }
  for (auto const base_class_name : ast_class->base_class_names()) {
    auto const base_class = GetResolved(base_class_name)->as<ast::Class>();
    FindInClass(name, base_class, founds);
  }
}

ast::NamedNode* NamespaceAnalyzer::FindResolved(ast::Expression* reference) {
  auto const it = reference_cache_.find(reference);
  return it == reference_cache_.end() ? nullptr : it->second;
}

NamespaceAnalyzer::AnalyzeNode* NamespaceAnalyzer::GetOrCreateNode(
    ast::NamedNode* member) {
  auto const it = map_.find(member);
  if (it != map_.end())
    return it->second;
  auto const node = new (zone()) AnalyzeNode(zone(), member);
  map_[member] = node;
  return node;
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

ast::NamedNode* NamespaceAnalyzer::GetResolved(ast::Expression* reference) {
  auto const present = FindResolved(reference);
  DCHECK(present);
  return present;
}

Maybe<ast::NamedNode*> NamespaceAnalyzer::Postpone(AnalyzeNode* node,
                                                   AnalyzeNode* using_node) {
  node->Use(using_node);
  unresolved_nodes_.insert(node);
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

  auto const base_class_node = GetOrCreateNode(base_class);
  if (!base_class_node->is_resolved()) {
    Postpone(GetOrCreateNode(clazz), base_class_node);
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
  auto const default_base_class_node = GetOrCreateNode(default_base_class);
  if (!default_base_class_node->is_resolved()) {
    Postpone(GetOrCreateNode(clazz), default_base_class_node);
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
    if (auto const present = container->FindMember(name)) {
      if (auto const alias = present->as<ast::Alias>()) {
        // Alias
        auto const alias_node = GetOrCreateNode(alias);
        if (!alias_node->is_resolved())
          return Postpone(context.node, alias_node);
        auto const resolved = FindResolved(alias->reference());
        if (!resolved)
          return Remember(reference, nullptr);
        DCHECK(!resolved->is<ast::Alias>());
        founds.insert(resolved);
      } else {
        founds.insert(present);
      }
    } else if (auto const clazz = container->as<ast::Class>()) {
      auto const clazz_node = GetOrCreateNode(clazz);
      if (!clazz_node->is_resolved())
        return Postpone(context.node, clazz_node);
      // TODO(eval1749) We should check |System.Object| base classes.
      for (auto const base_class_name : clazz->base_class_names()) {
        auto const resolved = FindResolved(base_class_name);
        if (!resolved)
          return Remember(reference, nullptr);
        auto const base_class = resolved->as<ast::Class>();
        FindInClass(name, base_class, &founds);
      }
    } else if (auto const ns_body = container->as<ast::NamespaceBody>()) {
      // Imports
      for (auto const import : ns_body->imports()) {
        auto const import_node = GetOrCreateNode(import);
        if (!import_node->is_resolved())
          return Postpone(context.node, import_node);
        auto const imported = GetResolved(import->reference());
        if (auto const present =
                imported->as<ast::ContainerNode>()->FindDirectMember(name)) {
          founds.insert(present);
        } else if (auto const clazz = imported->as<ast::Class>()) {
          auto const clazz_node = GetOrCreateNode(clazz);
          if (!clazz_node->is_resolved())
            return Postpone(context.node, clazz_node);
          // TODO(eval1749) We should check |System.Object| base classes.
          for (auto const base_class_name : clazz->base_class_names()) {
            auto const base_class =
                GetResolved(base_class_name)->as<ast::Class>();
            FindInClass(name, base_class, &founds);
          }
        }
      }
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
  if (auto const resolved = FindResolved(reference))
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

bool NamespaceAnalyzer::Run() {
  VisitNamespaceBody(session()->root_node());
  if (!session()->errors().empty())
    return false;
  if (unresolved_nodes_.empty())
    return true;
  // If there are odd number of self referenced node, |unresolved_nodes_.size()|
  // is odd number.
  for (auto const node : unresolved_nodes_) {
    if (auto const user = node->GetFirstUser())
      Error(ErrorCode::NameResolutionNameCycle, node->member(), user);
    else
      Error(ErrorCode::NameResolutionNameCycle, node->member());
  }
  return false;
}

// ast::Visitor
void NamespaceAnalyzer::VisitAlias(ast::Alias* alias) {
  auto const alias_node = GetOrCreateNode(alias);
  if (alias_node->is_resolved())
    return;
  ResolveContext context(alias_node, alias->parent()->parent());
  auto const result = ResolveReference(context, alias->reference());
  if (!result.has_value)
    return;
  if (result.value && !result.value->as<ast::ContainerNode>()) {
    Error(ErrorCode::NameResolutionAliasNeitherNamespaceNorType,
          alias->reference());
  }
  DidResolve(alias_node);
}

void NamespaceAnalyzer::VisitClass(ast::Class* clazz) {
  AnalyzeClass(clazz);
  clazz->AcceptForMembers(this);
}

void NamespaceAnalyzer::VisitImport(ast::Import* import) {
  auto const import_node = GetOrCreateNode(import);
  if (import_node->is_resolved())
    return;
  ResolveContext context(import_node, import->parent()->parent());
  auto const result = ResolveReference(context, import->reference());
  if (!result.has_value)
    return;
  if (result.value && !result.value->as<ast::ContainerNode>()) {
    Error(ErrorCode::NameResolutionImportNeitherNamespaceNorType,
          import->reference());
  }
  DidResolve(import_node);
}

// Builds namespace tree and schedule members to resolve.
void NamespaceAnalyzer::VisitNamespaceBody(ast::NamespaceBody* ns_body) {
  ns_body->AcceptForMembers(this);
}

}  // namespace compiler
}  // namespace elang
