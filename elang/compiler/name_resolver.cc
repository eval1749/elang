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
  private: hir::NamespaceMember* resolved_;
  private: NameResolver* const resolver_;

  public: ScopedResolver(NameResolver* resolver, ast::NamespaceMember* member);
  public: ~ScopedResolver();

  public: hir::NamespaceMember* Resolve();

  DISALLOW_COPY_AND_ASSIGN(ScopedResolver);
};

NameResolver::ScopedResolver::ScopedResolver(NameResolver* resolver,
                                             ast::NamespaceMember* member)
    : member_(member), resolved_(nullptr), resolver_(resolver) {
}

NameResolver::ScopedResolver::~ScopedResolver() {
  resolver_->running_stack_.pop_back();
  resolver_->running_set_.erase(member_);
  if (resolved_)
    return;
  resolver_->pending_set_.insert(member_);
  resolver_->pending_members_.push_back(member_);
}

hir::NamespaceMember* NameResolver::ScopedResolver::Resolve() {
  if (resolver_->running_set_.find(member_) != resolver_->running_set_.end()) {
    auto const another_member = resolver_->running_stack_.front();
    resolver_->session_->AddError(
        ErrorCode::NameResolutionNameCyclic,
        another_member->simple_name(),
        member_->simple_name());
    return nullptr;
  }
  resolver_->running_stack_.push_back(member_);
  resolver_->running_set_.insert(member_);
  if (auto const clazz = member_->as<ast::Class>())
    return resolved_ = resolver_->ResolveClass(clazz);
  if (auto const alias = member_->as<ast::Alias>())
    return resolved_ = resolver_->ResolveAlias(alias);
  if (member_->is<ast::Namespace>()) {
    // |member| is declared as namespace, but it is also declared as type.
    return nullptr;
  }
  NOTREACHED();
  return nullptr;
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

void NameResolver::BuildNamespaceTree(
    hir::Namespace* hir_enclosing_namespace,
    ast::Namespace* enclosing_namespace) {
  for (auto const body : enclosing_namespace->bodies()) {
    // TODO(eval1749) We should check alias name confliction.
    for (auto const member : body->members()) {
      auto const namespaze = member->as<ast::Namespace>();
      if (!namespaze) {
        pending_members_.push_back(member);
        pending_set_.insert(member);
        continue;
      }
      auto const simple_name = factory_->GetOrCreateSimpleName(
          namespaze->simple_name().string_data());
      if (auto const hir_member =
              hir_enclosing_namespace->FindMember(simple_name)) {
        if (auto const hir_namespace = hir_member->as<hir::Namespace>()) {
          BuildNamespaceTree(hir_namespace, namespaze);
          continue;
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
  }
}

hir::NamespaceMember* NameResolver::Resolve(ast::NamespaceMember* member) {
  if (pending_set_.find(member) != pending_set_.end())
    return nullptr;
  if (waiting_set_.find(member) != waiting_set_.end())
    return nullptr;
  auto const it = resolve_map_.find(member);
  if (it != resolve_map_.end())
    return it->second->as<hir::NamespaceMember>();
  ScopedResolver resolver(this, member);
  return resolver.Resolve();
}

hir::NamespaceMember* NameResolver::ResolveAlias(ast::Alias* alias) {
  return ResolveQualifiedName(alias->outer(), alias->namespace_body(),
                              alias->target_name());
}

hir::Class* NameResolver::ResolveClass(ast::Class* clazz) {
  // TODO(eval1749) We should check cycle in class declration. Outer classes
  // and base classes must not refer the class.
  std::vector<hir::Class*> base_classes;
  for (auto const base_class_name : clazz->base_class_names()) {
    auto const hir_member = ResolveQualifiedName(
        clazz->outer(), clazz->namespace_body(), base_class_name);
    if (!hir_member)
      return nullptr;
    auto const hir_base_class = hir_member->as<hir::Class>();
    if (!hir_base_class) {
      session_->AddError(ErrorCode::NameResolutionNameNotClass,
                         base_class_name.simple_name());
      return nullptr;
    }
    // TODO(eval1749) We should check accessibility of |hir_base_class|.
    base_classes.push_back(hir_base_class);
  }

  auto const hir_present = Resolve(clazz->outer());
  if (!hir_present)
    return nullptr;

  auto const hir_outer = hir_present->as<hir::Namespace>();
  if (!hir_outer) {
    session_->AddError(ErrorCode::NameResolutionNameNeitherNamespaceOrType,
                       clazz->outer()->simple_name());
    return nullptr;
  }

  // TODO(eval1749) We should check |base_classes.first()| is proper class
  // rather than |struct|, |interface|.
  // TODO(eval1749) We should record source code location into |hir::Class|.
  auto const new_class = factory_->NewClass(
    hir_outer,
    factory_->GetOrCreateSimpleName(clazz->simple_name().string_data()),
    base_classes);
  resolve_map_[clazz] = new_class;
  return new_class;
}

hir::NamespaceMember* NameResolver::ResolveLeftMostName(
    ast::Namespace* outer, ast::NamespaceBody* namespace_body,
    const Token& simple_name_token) {
  while (outer) {
    // TODO(eval1749) We should implement import.
    auto const present = outer->FindMember(simple_name_token);
    auto const alias = namespace_body->owner() == outer ?
        namespace_body->FindAlias(simple_name_token) : nullptr;
    if (present && alias) {
      auto const resolved1 = Resolve(present);
      if (!resolved1)
        return nullptr;
      auto const resolved2 = ResolveQualifiedName(
          outer, namespace_body, alias->target_name());
      if (!resolved2)
        return nullptr;
      if (resolved1 == resolved2)
        return resolved1;
      session_->AddError(ErrorCode::NameResolutionNameConflict,
                         simple_name_token);
    }
    if (present)
      return Resolve(present);
    if (alias)
      return ResolveQualifiedName(outer, namespace_body, alias->target_name());
    if (namespace_body->owner() == outer)
      namespace_body = namespace_body->outer();
    outer = outer->outer();
  }
  return nullptr;
}

hir::NamespaceMember* NameResolver::ResolveQualifiedName(
    ast::Namespace* outer, ast::NamespaceBody* namespace_body,
    const QualifiedName& name) {
  auto resolved = static_cast<hir::NamespaceMember*>(nullptr);
  for (auto const simple_name_token : name.simple_names()) {
    if (!resolved) {
      resolved = ResolveLeftMostName(outer, namespace_body, simple_name_token);
      if (!resolved)
        return nullptr;
      continue;
    }
    auto const namespaze = resolved->as<hir::Namespace>();
    if (!namespaze) {
      session_->AddError(
          ErrorCode::NameResolutionNameNeitherNamespaceOrType,
          simple_name_token);
      return nullptr;
    }
    auto const simple_name = factory_->GetOrCreateSimpleName(
          simple_name_token.string_data());
    resolved = namespaze->FindMember(simple_name);
    if (!resolved)
      return nullptr;
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

}  // namespace compiler
}  // namespace elang
