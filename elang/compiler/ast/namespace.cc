// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
Namespace::Namespace(Namespace* outer, const Token& keyword,
                     const Token& simple_name)
    : NamespaceMember(outer, keyword, simple_name), namespace_body_(nullptr) {
}

Namespace::~Namespace() {
  DCHECK(!namespace_body_);
}

NamespaceBody* Namespace::namespace_body() const {
  for (auto runner = this; runner; runner = runner->outer()) {
    if (runner->token().type() == TokenType::Namespace)
      return runner->namespace_body_;
  }
  NOTREACHED();
  return nullptr;
}

void Namespace::AddImport(const Token& keyword, const QualifiedName& name) {
  DCHECK_EQ(keyword.type(), TokenType::Using);
  DCHECK_EQ(token().type(), TokenType::Namespace);
  DCHECK_EQ(namespace_body_, bodies_.back());
  namespace_body_->AddImport(keyword, name);
}

void Namespace::AddMember(NamespaceMember* member) {
  DCHECK(namespace_body_);
  // We keep first member declaration.
  if (!FindMember(member->simple_name()))
    map_[member->simple_name().id()] = member;
  bodies_.back()->AddMember(member);
  members_.push_back(member);
}

void Namespace::Close() {
  DCHECK_EQ(namespace_body_, bodies_.back());
  for (auto const alias : namespace_body_->aliases())
    map_.erase(map_.find(alias->simple_name().id()));
  namespace_body_ = nullptr;
}

NamespaceMember* Namespace::FindMember(const Token& simple_name) {
  auto const it = map_.find(simple_name.id());
  return it == map_.end() ? nullptr : it->second;
}

void Namespace::Open(NamespaceBody* outer, SourceCode* source_code) {
  DCHECK(!namespace_body_);
  namespace_body_ = new NamespaceBody(outer, this, source_code);
  bodies_.push_back(namespace_body_);
}

// NamespaceMember
Namespace* Namespace::ToNamespace() {
  return this;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
