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
    : NamespaceMember(outer, keyword, simple_name), body_(nullptr) {
  DCHECK_EQ(keyword.type(), TokenType::Namespace);
}

Namespace::~Namespace() {
  DCHECK(!body_);
}

void Namespace::AddImport(const Token& keyword, const QualifiedName& name) {
  DCHECK_EQ(keyword.type(), TokenType::Using);
  DCHECK_EQ(token().type(), TokenType::Namespace);
  DCHECK_EQ(body_, bodies_.back());
  body_->AddImport(keyword, name);
}

void Namespace::AddMember(NamespaceMember* member) {
  DCHECK(body_);
  // We keep first member declaration.
  if (!FindMember(member->simple_name()))
    map_[member->simple_name().id()] = member;
  bodies_.back()->AddMember(member);
}

void Namespace::Close() {
  DCHECK_EQ(body_, bodies_.back());
  for (auto const alias : body_->aliases())
    map_.erase(map_.find(alias->simple_name().id()));
  body_ = nullptr;
}

NamespaceMember* Namespace::FindMember(const Token& simple_name) {
  auto const it = map_.find(simple_name.id());
  return it == map_.end() ? nullptr : it->second;
}

void Namespace::Open(SourceCode* source_code) {
  DCHECK(!body_);
  body_ = new NamespaceBody(this, source_code);
  bodies_.push_back(body_);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
