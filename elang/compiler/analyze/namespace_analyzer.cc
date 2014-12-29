// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/namespace_analyzer.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
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
// ScopedResolver
//
class NamespaceAnalyzer::ScopedResolver final {
 public:
  ScopedResolver(NamespaceAnalyzer* resolver, ast::NamespaceMember* member);
  ~ScopedResolver();

  Maybe<ast::NamespaceMember*> Resolve();
  Maybe<ast::NamespaceMember*> ResolveInternal();

 private:
  ast::NamespaceMember* member_;
  NamespaceAnalyzer* const resolver_;

  DISALLOW_COPY_AND_ASSIGN(ScopedResolver);
};

NamespaceAnalyzer::ScopedResolver::ScopedResolver(NamespaceAnalyzer* resolver,
                                                  ast::NamespaceMember* member)
    : member_(member), resolver_(resolver) {
  if (resolver_->running_set_.find(member_) == resolver_->running_set_.end()) {
    resolver_->running_stack_.push_back(member_);
    resolver_->running_set_.insert(member_);
    return;
  }

  auto const another_member =
      resolver_->running_stack_.size() == 1
          ? resolver_->running_stack_.front()
          : resolver_->running_stack_[resolver_->running_stack_.size() - 2];
  resolver_->session_->AddError(ErrorCode::NameResolutionNameCycle,
                                another_member->name(), member_->name());
  member_ = nullptr;
}

NamespaceAnalyzer::ScopedResolver::~ScopedResolver() {
  if (!member_)
    return;
  resolver_->running_stack_.pop_back();
  resolver_->running_set_.erase(member_);
}

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::ScopedResolver::Resolve() {
  auto const result = ResolveInternal();
  if (!result.has_value)
    resolver_->Schedule(member_);
  return result;
}

Maybe<ast::NamespaceMember*>
NamespaceAnalyzer::ScopedResolver::ResolveInternal() {
  if (!member_)
    return Maybe<ast::NamespaceMember*>(nullptr);
#if 0
  if (resolver_->pending_set_.find(member_) != resolver_->pending_set_.end())
    return Maybe<ast::NamespaceMember*>();
  if (resolver_->waiting_set_.find(member_) != resolver_->waiting_set_.end())
    return Maybe<ast::NamespaceMember*>();
#endif
  if (auto const clazz = member_->as<ast::Class>())
    return resolver_->FixClass(clazz);
  if (auto const alias = member_->as<ast::Alias>()) {
    if (auto const target = alias->target())
      return resolver_->Resolve(target);
    return Maybe<ast::NamespaceMember*>(nullptr);
  }
  if (member_->ToNamespace()) {
    return Maybe<ast::NamespaceMember*>(member_);
  }
  NOTREACHED();
  return Maybe<ast::NamespaceMember*>(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
NamespaceAnalyzer::NamespaceAnalyzer(CompilationSession* session)
    : session_(session) {
}

NamespaceAnalyzer::~NamespaceAnalyzer() {
}

void NamespaceAnalyzer::BindAlias(ast::Alias* alias) {
  DCHECK(!alias->target());
  auto const target = ResolveQualifiedName(
      alias->outer(), alias->namespace_body()->outer(), alias->target_name());
  if (!target) {
    session_->AddError(ErrorCode::NameResolutionAliasNoTarget, alias->name());
    return;
  }
  if (!target->as<ast::Namespace>()) {
    session_->AddError(ErrorCode::NameResolutionNameNeitherNamespaceNorType,
                       alias->target_name().simple_name());
  }
  alias->BindTo(target);
}

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::FixClass(ast::Class* clazz) {
  if (clazz->is_fixed())
    return Maybe<ast::NamespaceMember*>(clazz);

  // Resolve enclosing namespace or class.
  auto const resolution = Resolve(clazz->outer());
  if (!resolution.has_value || !resolution.value)
    return resolution;

  auto const outer = resolution.value->as<ast::Namespace>();
  if (!outer) {
    session_->AddError(ErrorCode::NameResolutionNameNeitherNamespaceNorType,
                       clazz->outer()->name());
    return Maybe<ast::NamespaceMember*>(nullptr);
  }

  // Resolve base classes
  auto has_value = true;
  auto is_base_classes_valid = true;
  std::vector<ast::Class*> base_classes;
  for (auto const base_class_name : clazz->base_class_names()) {
    auto const found = ResolveQualifiedName(
        clazz->outer(), clazz->namespace_body(), base_class_name);
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
    auto const base_class = resolution.value->as<ast::Class>();
    if (!base_class) {
      session_->AddError(ErrorCode::NameResolutionNameNotClass,
                         base_class_name.simple_name());
      is_base_classes_valid = false;
      continue;
    }

    // |base_class| must be an interface except for first one.
    if (base_class->token() == TokenType::Class) {
      if (!base_classes.empty()) {
        session_->AddError(ErrorCode::NameResolutionNameNotInterface,
                           base_class_name.simple_name());
        is_base_classes_valid = false;
        continue;
      }
    } else if (base_class->token()->type() != TokenType::Interface) {
      session_->AddError(
          base_classes.empty()
              ? ErrorCode::NameResolutionNameNeitherClassNortInterface
              : ErrorCode::NameResolutionNameNotInterface,
          base_class_name.simple_name());
      is_base_classes_valid = false;
      continue;
    }

    if (base_class == outer || outer->IsDescendantOf(base_class)) {
      session_->AddError(ErrorCode::NameResolutionClassContaining,
                         base_class_name.simple_name(), clazz->name());
      is_base_classes_valid = false;
      continue;
    }

    // TODO(eval1749) Check |base_class| isn't |final|.
    // TODO(eval1749) We should check accessibility of |base_class|.
    base_classes.push_back(base_class);
  }

  if (!is_base_classes_valid)
    return Maybe<ast::NamespaceMember*>(nullptr);
  if (!has_value)
    return Maybe<ast::NamespaceMember*>();

  clazz->BindBaseClasses(base_classes);
  return Maybe<ast::NamespaceMember*>(clazz);
}

// Builds namespace tree and schedule members to resolve.
void NamespaceAnalyzer::BindMembers(ast::Namespace* enclosing_namespace) {
  for (auto const body : enclosing_namespace->bodies()) {
    for (auto const alias : body->aliases())
      BindAlias(alias);
    for (auto const member : body->members()) {
      if (auto const clazz = member->as<ast::Class>()) {
        ScheduleClassTree(clazz);
        continue;
      }
      if (auto const namespaze = member->ToNamespace())
        BindMembers(namespaze);
    }
  }
}

Maybe<ast::NamespaceMember*> NamespaceAnalyzer::Resolve(
    ast::NamespaceMember* member) {
  ScopedResolver resolver(this, member);
  return resolver.Resolve();
}

ast::NamespaceMember* NamespaceAnalyzer::ResolveLeftMostName(
    ast::Namespace* outer,
    ast::NamespaceBody* alias_namespace,
    const QualifiedName& name) {
  DCHECK(outer);
  const auto& simple_name = name.simple_names()[0];
  while (outer) {
    auto const present = outer->FindMember(simple_name);
    if (alias_namespace && alias_namespace->owner() == outer->ToNamespace()) {
      // TODO(eval1749) We should implement import.
      if (auto const alias = alias_namespace->FindAlias(simple_name)) {
        auto const target = alias->target();
        if (!target)
          return nullptr;
        if (present && target != present) {
          session_->AddError(target ? ErrorCode::NameResolutionNameAmbiguous
                                    : ErrorCode::NameResolutionAliasNoTarget,
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

ast::NamespaceMember* NamespaceAnalyzer::ResolveQualifiedName(
    ast::Namespace* outer,
    ast::NamespaceBody* alias_namespace,
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
      session_->AddError(ErrorCode::NameResolutionNameNeitherNamespaceNorType,
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

bool NamespaceAnalyzer::Run() {
  BindMembers(session_->global_namespace());
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

void NamespaceAnalyzer::ScheduleClassTree(ast::Class* clazz) {
  if (!clazz->is_fixed())
    Schedule(clazz);
  for (auto const body : clazz->bodies()) {
    for (auto const member : body->members()) {
      if (auto const inner_class = member->as<ast::Class>())
        ScheduleClassTree(inner_class);
      else
        Schedule(member);
    }
  }
}

void NamespaceAnalyzer::Schedule(ast::NamespaceMember* member) {
  if (!member->is<ast::Class>() && !member->is<ast::Alias>())
    return;
  pending_set_.insert(member);
  pending_members_.push_back(member);
}

}  // namespace compiler
}  // namespace elang
