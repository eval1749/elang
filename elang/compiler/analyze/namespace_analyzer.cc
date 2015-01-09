// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/namespace_analyzer.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/import.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

///////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer::AnalyzeNode
// Represents a dependency graph node.
//
class NamespaceAnalyzer::AnalyzeNode final {
 public:
  explicit AnalyzeNode(ast::NamespaceMember* member);
  ~AnalyzeNode();

  bool is_resolved() const { return is_resolved_; }
  ast::NamespaceMember* member() const { return member_; }
  const std::unordered_set<AnalyzeNode*> uses() const { return uses_; }
  const std::unordered_set<AnalyzeNode*> users() const { return users_; }

  Token* GetFirstUserName() const;
  void RemoveUse(AnalyzeNode* node);
  void Resolved();
  void Use(AnalyzeNode* node);

 private:
  bool is_resolved_;
  ast::NamespaceMember* const member_;
  std::unordered_set<AnalyzeNode*> uses_;
  std::unordered_set<AnalyzeNode*> users_;

  DISALLOW_COPY_AND_ASSIGN(AnalyzeNode);
};

NamespaceAnalyzer::AnalyzeNode::AnalyzeNode(ast::NamespaceMember* member)
    : is_resolved_(!!member->ToNamespace()), member_(member) {
}

NamespaceAnalyzer::AnalyzeNode::~AnalyzeNode() {
}

Token* NamespaceAnalyzer::AnalyzeNode::GetFirstUserName() const {
  auto node = static_cast<AnalyzeNode*>(nullptr);
  for (auto const runner : users_) {
    if (!node || node->member()->name()->location().start_offset() >
        runner->member()->name()->location().start_offset()) {
      node = runner;
    }
  }
  return node ? node->member()->name() : nullptr;
}

void NamespaceAnalyzer::AnalyzeNode::RemoveUse(AnalyzeNode* node) {
  uses_.erase(node);
  node->users_.erase(this);
}

void NamespaceAnalyzer::AnalyzeNode::Resolved() {
  DCHECK(!is_resolved_);
  is_resolved_ = true;
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
  ast::NamespaceBody* alias_space;
  ast::MemberAccess* member_access;
  ast::Namespace* name_space;
  AnalyzeNode* node;

  ResolveContext(AnalyzeNode* node,
                 ast::Namespace* name_space,
                 ast::NamespaceBody* alias_space);
};

// Resolve name in |namespace|.
NamespaceAnalyzer::ResolveContext::ResolveContext(
    AnalyzeNode* node,
    ast::Namespace* name_space,
    ast::NamespaceBody* alias_space)
    : alias_space(alias_space), member_access(nullptr), name_space(name_space),
      node(node) {
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
NamespaceAnalyzer::NamespaceAnalyzer(CompilationSession* session,
                                     NameResolver* resolver)
    : resolver_(resolver), session_(session) {
  DCHECK(resolver_);
  DCHECK(session_);
}

NamespaceAnalyzer::~NamespaceAnalyzer() {
  for (const auto& pair : map_)
    delete pair.second;
}

bool NamespaceAnalyzer::AnalyzeAlias(ast::Alias* alias) {
  auto const alias_node = GetOrCreateNode(alias);
  if (alias_node->is_resolved())
    return true;
  ResolveContext context(alias_node, alias->outer(),
                         alias->namespace_body()->outer());
  auto const result = ResolveReference(context, alias->reference());
  if (!result.has_value)
    return false;
  if (result.value) {
    auto const name_space = result.value->as<ast::Namespace>();
    if (!name_space) {
      session_->AddError(ErrorCode::NameResolutionAliasNeitherNamespaceNorType,
                         alias->reference()->token());
    }
  }
  Resolved(alias_node);
  return true;
}

bool NamespaceAnalyzer::AnalyzeClass(ast::Class* clazz) {
  auto const class_node = GetOrCreateNode(clazz);
  if (class_node->is_resolved())
    return true;

  auto has_value = true;

  // Check enclosing class
  if (auto const enclosing_class = clazz->outer()->as<ast::Class>()) {
    auto const enclosing_class_node = GetOrCreateNode(enclosing_class);
    if (!enclosing_class_node->is_resolved()) {
      Postpone(class_node, enclosing_class_node);
      has_value = false;
    }
  }

  // Resolve base classes
  auto are_base_classes_valid = true;
  auto nth = 0;
  ResolveContext context(class_node, clazz->outer(), clazz->namespace_body());
  for (auto const base_class_name : clazz->base_class_names()) {
    ++nth;
    auto const result = ResolveReference(context, base_class_name);
    if (!result.has_value) {
      has_value = false;
      continue;
    }
    if (!result.value) {
      are_base_classes_valid = false;
      continue;
    }
    auto const base_class = result.value->as<ast::Class>();
    if (!base_class) {
      session_->AddError(ErrorCode::NameResolutionNameNotClass,
                         base_class_name->token());
      are_base_classes_valid = false;
      continue;
    }
    // |base_class| must be an interface except for first one.
    if (base_class->token() == TokenType::Class) {
      if (nth >= 2) {
        session_->AddError(ErrorCode::NameResolutionNameNotInterface,
                           base_class_name->token());
        are_base_classes_valid = false;
        continue;
      }
    } else if (base_class->token()->type() != TokenType::Interface) {
      session_->AddError(
          nth == 1 ? ErrorCode::NameResolutionNameNeitherClassNortInterface
                   : ErrorCode::NameResolutionNameNotInterface,
          base_class_name->token());
      are_base_classes_valid = false;
      continue;
    }

    // TODO(eval1749) Check |base_class| isn't |final|.
    // TODO(eval1749) We should check accessibility of |base_class|.

    if (base_class == clazz->outer() ||
        clazz->outer()->IsDescendantOf(base_class)) {
      session_->AddError(ErrorCode::NameResolutionClassContaining,
                         base_class_name->token(), clazz->name());
      are_base_classes_valid = false;
      continue;
    }

    auto const base_class_node = GetOrCreateNode(base_class);
    if (!base_class_node->is_resolved()) {
      Postpone(class_node, base_class_node);
      has_value = false;
    }
  }

  if (!are_base_classes_valid) {
    Resolved(class_node);
    return true;
  }

  if (!has_value)
    return false;

  Resolved(class_node);
  return true;
}

bool NamespaceAnalyzer::AnalyzeImport(ast::Import* import) {
  auto const import_node = GetOrCreateNode(import);
  if (import_node->is_resolved())
    return true;
  ResolveContext context(import_node, import->outer(),
                         import->namespace_body()->outer());
  auto const result = ResolveReference(context, import->reference());
  if (!result.has_value)
    return false;
  if (result.value) {
    auto const name_space = result.value->as<ast::Namespace>();
    if (!name_space) {
      session_->AddError(ErrorCode::NameResolutionImportNeitherNamespaceNorType,
                         import->reference()->token());
    }
  }
  Resolved(import_node);
  return true;
}

// Builds namespace tree and schedule members to resolve.
bool NamespaceAnalyzer::AnalyzeNamespace(ast::Namespace* enclosing_namespace) {
  DCHECK(enclosing_namespace->ToNamespace());
  for (auto const body : enclosing_namespace->bodies()) {
    for (auto const import : body->imports())
      AnalyzeImport(import);
    for (auto const alias : body->aliases())
      AnalyzeAlias(alias);
    for (auto const member : body->members()) {
      AnalyzeNamespaceMember(member);
    }
  }
  return true;
}

bool NamespaceAnalyzer::AnalyzeNamespaceMember(ast::NamespaceMember* member) {
  if (auto const alias = member->as<ast::Alias>())
    return AnalyzeAlias(alias);
  if (auto const clazz = member->as<ast::Class>()) {
    auto const result = AnalyzeClass(clazz);
    for (auto const body : clazz->bodies()) {
      for (auto const member : body->members())
        AnalyzeNamespaceMember(member);
    }
    return result;
  }
  if (auto const import = member->as<ast::Import>())
    return AnalyzeImport(import);
  if (auto const namespaze = member->as<ast::Namespace>())
    return AnalyzeNamespace(namespaze);
  NOTREACHED();
  return false;
}

// Find |name| in class tree rooted by |clazz|.
std::unordered_set<ast::NamespaceMember*> NamespaceAnalyzer::FindInClass(
    Token* name,
    ast::Class* clazz) {
  DCHECK(GetOrCreateNode(clazz)->is_resolved());
  if (auto const present = clazz->FindMember(name))
    return {present};
  std::unordered_set<ast::NamespaceMember*> founds;
  for (auto const base_class_name : clazz->base_class_names()) {
    auto const base_class = GetResolved(base_class_name)->as<ast::Class>();
    for (auto const present : FindInClass(name, base_class))
      founds.insert(present);
  }
  return founds;
}

ast::NamespaceMember* NamespaceAnalyzer::FindResolved(
    ast::Expression* reference) {
  auto const it = reference_cache_.find(reference);
  return it == reference_cache_.end() ? nullptr : it->second;
}

NamespaceAnalyzer::AnalyzeNode* NamespaceAnalyzer::GetOrCreateNode(
    ast::NamespaceMember* member) {
  auto const it = map_.find(member);
  if (it != map_.end())
    return it->second;
  auto const node = new AnalyzeNode(member);
  map_[member] = node;
  return node;
}

ast::NamespaceMember* NamespaceAnalyzer::GetResolved(
    ast::Expression* reference) {
  auto const present = FindResolved(reference);
  DCHECK(present);
  return present;
}

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::Postpone(
    AnalyzeNode* node,
    AnalyzeNode* using_node) {
  node->Use(using_node);
  unresolved_nodes_.insert(node);
  return Maybe<ast::NamespaceMember*>();
}

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::Remember(
    ast::Expression* reference,
    ast::NamespaceMember* member) {
  DCHECK(reference_cache_.find(reference) == reference_cache_.end());
  reference_cache_[reference] = member;
  if (member)
    resolver_->Resolved(reference, member);
  return Maybe<ast::NamespaceMember*>(member);
}

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::ResolveMemberAccess(
    const ResolveContext& start_context,
    ast::MemberAccess* reference) {
  ResolveContext context(start_context);
  context.member_access = reference;
  auto resolved = static_cast<ast::NamespaceMember*>(nullptr);
  for (auto const component : reference->components()) {
    if (resolved) {
      auto const namespaze = resolved->as<ast::Namespace>();
      if (!namespaze) {
        session_->AddError(ErrorCode::NameResolutionNameNeitherNamespaceNorType,
                           component->token(), reference->token());
        return Remember(reference, nullptr);
      }
      context.name_space = namespaze;
      context.alias_space = nullptr;
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

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::ResolveNameReference(
    const ResolveContext& context,
    ast::NameReference* reference) {
  auto const name = reference->name();
  auto name_space = context.name_space;
  auto alias_space = context.alias_space;
  while (name_space) {
    std::unordered_set<ast::NamespaceMember*> founds;
    if (auto const present = name_space->FindMember(name)) {
      DCHECK(!present->is<ast::Alias>());
      founds.insert(present);
    } else if (auto const clazz = name_space->as<ast::Class>()) {
      auto const clazz_node = GetOrCreateNode(clazz);
      if (!clazz_node->is_resolved())
        return Postpone(context.node, clazz_node);
      for (auto const base_class_name : clazz->base_class_names()) {
        auto const resolved = FindResolved(base_class_name);
        if (!resolved)
          return Remember(reference, nullptr);
        auto const base_class = resolved->as<ast::Class>();
        for (auto const present : FindInClass(name, base_class))
          founds.insert(present);
      }
    }

    if (alias_space && name_space->ToNamespace() &&
        alias_space->owner() == name_space) {
      // Alias
      if (auto const alias = alias_space->FindAlias(name)) {
        auto const alias_node = GetOrCreateNode(alias);
        if (!alias_node->is_resolved())
          return Postpone(context.node, alias_node);
        auto const resolved = FindResolved(alias->reference());
        if (!resolved)
          return Remember(reference, nullptr);
        DCHECK(!resolved->is<ast::Alias>());
        founds.insert(resolved);
      }

      // Imports
      for (auto const import : alias_space->imports()) {
        auto const import_node = GetOrCreateNode(import);
        if (!import_node->is_resolved())
          return Postpone(context.node, import_node);
        auto const imported = GetResolved(import->reference());
        if (auto const present =
                imported->as<ast::Namespace>()->FindMember(name)) {
          founds.insert(present);
        } else if (auto const clazz = imported->as<ast::Class>()) {
          auto const clazz_node = GetOrCreateNode(clazz);
          if (!clazz_node->is_resolved())
            return Postpone(context.node, clazz_node);
          for (auto const base_class_name : clazz->base_class_names()) {
            auto const base_class =
                GetResolved(base_class_name)->as<ast::Class>();
            for (auto const present : FindInClass(name, base_class))
              founds.insert(present);
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
      session_->AddError(ErrorCode::NameResolutionNameAmbiguous, name);
      return Remember(reference, nullptr);
    }

    // Current name space doesn't contain |name|. Ge enclosing name space.
    if (alias_space && alias_space->owner() == name_space)
      alias_space = alias_space->outer();
    name_space = name_space->outer();
  }
  if (context.member_access)
    session_->AddError(ErrorCode::NameResolutionNameNotResolved, name);
  else
    session_->AddError(ErrorCode::NameResolutionNameNotFound, name);
  return Remember(reference, nullptr);
}

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::ResolveReference(
    const ResolveContext& context,
    ast::Expression* reference) {
  if (auto const resolved = FindResolved(reference))
    return Maybe<ast::NamespaceMember*>(resolved);
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
  return Maybe<ast::NamespaceMember*>(nullptr);
}

void NamespaceAnalyzer::Resolved(AnalyzeNode* node) {
  node->Resolved();
  unresolved_nodes_.erase(node);
  std::vector<AnalyzeNode*> users;
  for (auto const user : node->users())
    users.push_back(user);
  for (auto const user : users) {
    user->RemoveUse(node);
    if (!user->uses().empty())
      continue;
    AnalyzeNamespaceMember(user->member());
  }
}

bool NamespaceAnalyzer::Run() {
  AnalyzeNamespace(session_->global_namespace());
  if (!session_->errors().empty())
    return false;
  if (unresolved_nodes_.empty())
    return true;
  // If there are odd number of self referenced node, |unresolved_nodes_.size()|
  // is odd number.
  for (auto const node : unresolved_nodes_) {
    session_->AddError(ErrorCode::NameResolutionNameCycle,
                       node->member()->name(),
                       node->GetFirstUserName());
  }
  return false;
}

}  // namespace compiler
}  // namespace elang
