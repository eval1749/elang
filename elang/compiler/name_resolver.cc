// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/name_resolver.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/hir/class.h"
#include "elang/hir/factory.h"
#include "elang/hir/namespace.h"
#include "elang/hir/namespace_member.h"
#include "elang/hir/simple_name.h"

namespace elang {
namespace compiler {

///////////////////////////////////////////////////////////////////////
//
// ScopedResolver
//
class NameResolver::ScopedResolver final {
  private: ast::NamespaceMember* member_;
  private: NameResolver* const resolver_;

  public: ScopedResolver(NameResolver* resolver, ast::NamespaceMember* member);
  public: ~ScopedResolver();

  public: Maybe<hir::NamespaceMember*> Resolve();
  public: Maybe<hir::NamespaceMember*> ResolveInternal();

  DISALLOW_COPY_AND_ASSIGN(ScopedResolver);
};

NameResolver::ScopedResolver::ScopedResolver(NameResolver* resolver,
                                             ast::NamespaceMember* member)
    : member_(member), resolver_(resolver) {
  if (resolver_->running_set_.find(member_) == resolver_->running_set_.end()) {
    resolver_->running_stack_.push_back(member_);
    resolver_->running_set_.insert(member_);
    return;
  }

  auto const another_member = resolver_->running_stack_.size() == 1 ?
      resolver_->running_stack_.front() :
      resolver_->running_stack_[resolver_->running_stack_.size() - 2];
  resolver_->session_->AddError(
      ErrorCode::NameResolutionNameCycle,
      another_member->simple_name(),
      member_->simple_name());
  member_ = nullptr;
}

NameResolver::ScopedResolver::~ScopedResolver() {
  if (!member_)
    return;
  resolver_->running_stack_.pop_back();
  resolver_->running_set_.erase(member_);
}

Maybe<hir::NamespaceMember*> NameResolver::ScopedResolver::Resolve() {
  auto const result = ResolveInternal();
  if (!result.has_value)
    resolver_->Schedule(member_);
  return result;
}

Maybe<hir::NamespaceMember*> NameResolver::ScopedResolver::ResolveInternal() {
  if (!member_)
    return Maybe<hir::NamespaceMember*>(nullptr);
  auto const it = resolver_->resolve_map_.find(member_);
  if (it != resolver_->resolve_map_.end())
    return maybe(it->second->as<hir::NamespaceMember>());
#if 0
  if (resolver_->pending_set_.find(member_) != resolver_->pending_set_.end())
    return Maybe<hir::NamespaceMember*>();
  if (resolver_->waiting_set_.find(member_) != resolver_->waiting_set_.end())
    return Maybe<hir::NamespaceMember*>();
#endif
  if (auto const clazz = member_->as<ast::Class>())
    return resolver_->BindClass(clazz);
  if (auto const alias = member_->as<ast::Alias>()) {
    if (auto const target = alias->target())
      return resolver_->Resolve(target);
    return Maybe<hir::NamespaceMember*>(nullptr);
  }
  if (member_->ToNamespace()) {
    // |member_| is declared as namespace, but it is also declared as type.
    return Maybe<hir::NamespaceMember*>(nullptr);
  }
  NOTREACHED();
  return Maybe<hir::NamespaceMember*>(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// NameResolver
//
NameResolver::NameResolver(CompilationSession* session, hir::Factory* factory)
    : factory_(factory), session_(session) {
}

NameResolver::~NameResolver() {
}

void NameResolver::BindAlias(ast::Alias* alias) {
  DCHECK(!alias->target());
  auto const target = ResolveQualifiedName(
      alias->outer(), alias->alias_declaration_space()->outer(),
      alias->target_name());
  if (!target) {
    session_->AddError(ErrorCode::NameResolutionAliasNoTarget,
                       alias->simple_name());
    return;
  }
  if (!target->as<ast::Namespace>()) {
    session_->AddError(ErrorCode::NameResolutionNameNeitherNamespaceOrType,
                       alias->target_name().simple_name());
  }
  alias->BindTo(target);
}

Maybe<hir::NamespaceMember*> NameResolver::BindClass(ast::Class* clazz) {
  // Resolve enclosing namespace or class.
  auto const resolution = Resolve(clazz->outer());
  if (!resolution.has_value || !resolution.value)
    return resolution;

  auto const hir_outer = resolution.value->as<hir::Namespace>();
  if (!hir_outer) {
    session_->AddError(ErrorCode::NameResolutionNameNeitherNamespaceOrType,
                       clazz->outer()->simple_name());
    return Maybe<hir::NamespaceMember*>(nullptr);
  }

  // Resolve base classes
  auto has_value = true;
  auto is_base_classes_valid = true;
  std::vector<hir::Class*> base_classes;
  for (auto const base_class_name : clazz->base_class_names()) {
    auto const found = ResolveQualifiedName(
        clazz->outer(), clazz->alias_declaration_space(), base_class_name);
    if (!found) {
      is_base_classes_valid = false;
      continue;
    }
    auto const resolution = Resolve(found);
    if (!resolution.has_value) {
      has_value = false;
      continue;
    }
    if (!resolution.value) {
      is_base_classes_valid = false;
      continue;
    }
    auto const hir_base_class = resolution.value->as<hir::Class>();
    if (!hir_base_class) {
      session_->AddError(ErrorCode::NameResolutionNameNotClass,
                         base_class_name.simple_name());
      is_base_classes_valid = false;
      continue;
    }
    if (hir_base_class == hir_outer ||
        hir_outer->IsDescendantOf(hir_base_class)) {
      session_->AddError(ErrorCode::NameResolutionClassContaining,
          base_class_name.simple_name(),
          clazz->simple_name());
      is_base_classes_valid = false;
      continue;
    }
    // TODO(eval1749) Check |hir_base_class| isn't |final|.
    // TODO(eval1749) We should check accessibility of |hir_base_class|.
    base_classes.push_back(hir_base_class);
  }

  if (!is_base_classes_valid)
    return Maybe<hir::NamespaceMember*>(nullptr);
  if (!has_value)
    return Maybe<hir::NamespaceMember*>();

  // TODO(eval1749) We should check |base_classes.first()| is proper class
  // rather than |struct|, |interface|.
  // TODO(eval1749) We should record source code location into |hir::Class|.
  auto const new_class = factory_->NewClass(
    hir_outer,
    factory_->GetOrCreateSimpleName(clazz->simple_name().string_data()),
    base_classes);
  resolve_map_[clazz] = new_class;
  hir_outer->AddMember(new_class);
  return Maybe<hir::NamespaceMember*>(new_class);
}

// Build HIR namespace object and schedule namespace members to resolve.
void NameResolver::BuildNamespace(hir::Namespace* hir_enclosing_namespace,
                                  ast::Namespace* namespaze) {
  auto const simple_name = factory_->GetOrCreateSimpleName(
      namespaze->simple_name().string_data());
  auto const hir_member = hir_enclosing_namespace->FindMember(simple_name);
  if (hir_member) {
    if (auto const hir_namespace = hir_member->ToNamespace()) {
      BuildNamespaceTree(hir_namespace, namespaze);
      return;
    }
    session_->AddError(ErrorCode::NameResolutionNameNotNamespace,
                       namespaze->simple_name());
    return;
  }

  auto const new_namespace = factory_->NewNamespace(
      hir_enclosing_namespace, simple_name);
  hir_enclosing_namespace->AddMember(new_namespace);
  BuildNamespaceTree(new_namespace, namespaze);
}

// Builds namespace tree and schedule members to resolve.
void NameResolver::BuildNamespaceTree(
    hir::Namespace* hir_enclosing_namespace,
    ast::Namespace* enclosing_namespace) {
  resolve_map_[enclosing_namespace] = hir_enclosing_namespace;
  for (auto const body : enclosing_namespace->bodies()) {
    for (auto const alias : body->aliases())
      BindAlias(alias);
    for (auto const member : body->members()) {
      if (auto const clazz = member->as<ast::Class>()) {
        ScheduleClassTree(clazz);
        continue;
      }
      if (auto const namespaze = member->ToNamespace())
        BuildNamespace(hir_enclosing_namespace, namespaze);
    }
  }
}

Maybe<hir::NamespaceMember*> NameResolver::Resolve(
    ast::NamespaceMember* member) {
  ScopedResolver resolver(this, member);
  return resolver.Resolve();
}

ast::NamespaceMember* NameResolver::ResolveLeftMostName(
    ast::Namespace* outer, ast::NamespaceBody* alias_namespace,
    const QualifiedName& name) {
  DCHECK(outer);
  const auto& simple_name = name.simple_names()[0];
  while (outer) {
    auto const present = outer->FindMember(simple_name);
    if (alias_namespace && alias_namespace->owner() == outer) {
      // TODO(eval1749) We should implement import.
      if (auto const alias = alias_namespace->FindAlias(simple_name)) {
        auto const target = alias->target();
        if (!target)
          return nullptr;
        if (present && target != present) {
          session_->AddError(
               target ? ErrorCode::NameResolutionNameAmbiguous :
                        ErrorCode::NameResolutionAliasNoTarget,
               simple_name);
        }
        return target;
      }
      alias_namespace = alias_namespace->outer();
    }

    if (present)
      return present;
    outer = outer->outer();
  }
  session_->AddError(ErrorCode::NameResolutionNameNotFound, simple_name);
  return nullptr;
}

ast::NamespaceMember* NameResolver::ResolveQualifiedName(
    ast::Namespace* outer, ast::NamespaceBody* alias_namespace,
    const QualifiedName& name) {
  DCHECK(outer);
  ast::NamespaceMember* resolved = nullptr;
  for (auto const simple_name : name.simple_names()) {
    if (!resolved) {
      resolved = ResolveLeftMostName(outer, alias_namespace, name);
      if (!resolved)
        return nullptr;
      continue;
    }
    auto const namespaze = resolved->as<ast::Namespace>();
    if (!namespaze) {
      session_->AddError(
          ErrorCode::NameResolutionNameNeitherNamespaceOrType,
          simple_name);
      return nullptr;
    }
    resolved = namespaze->FindMember(simple_name);
    if (!resolved) {
      session_->AddError(ErrorCode::NameResolutionNameNotFound, simple_name);
      return nullptr;
    }
  }
  DCHECK(resolved);
  return resolved;
}

bool NameResolver::Run() {
  BuildNamespaceTree(factory_->global_namespace(),
                     session_->global_namespace());
  do {
    DCHECK(waiting_set_.empty());
    waiting_set_.swap(pending_set_);
    decltype(pending_members_) waiting_members;
    waiting_members.swap(pending_members_);
    for (auto member : waiting_members) {
      waiting_set_.erase(member);
      Resolve(member);
    }
    if (!session_->errors().empty())
      return false;
  } while (!pending_set_.empty());
  return true;
}

void NameResolver::ScheduleClassTree(ast::Class* clazz) {
  Schedule(clazz);
  for (auto const member : clazz->members()) {
    if (auto const inner_class = member->as<ast::Class>())
      ScheduleClassTree(inner_class);
    else
      Schedule(member);
  }
}

void NameResolver::Schedule(ast::NamespaceMember* member) {
  if (!member->is<ast::Class>() && !member->is<ast::Alias>())
    return;
  pending_set_.insert(member);
  pending_members_.push_back(member);
}

}  // namespace compiler
}  // namespace elang
