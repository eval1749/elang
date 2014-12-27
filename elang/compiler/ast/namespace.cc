// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
Namespace::Namespace(NamespaceBody* namespace_body, Token* keyword,
                     Token* simple_name)
    : NamespaceMember(namespace_body, keyword, simple_name) {
}

Namespace::~Namespace() {
}

void Namespace::AddMember(NamespaceMember* member) {
  DCHECK_EQ(this, member->owner());
  DCHECK(!member->is<Alias>());
  // We keep first member declaration.
  if (FindMember(member->simple_name()))
    return;
  map_[member->simple_name()->simple_name()] = member;
}

void Namespace::AddNamespaceBody(NamespaceBody* namespace_body) {
  bodies_.push_back(namespace_body);
}

NamespaceMember* Namespace::FindMember(hir::SimpleName* simple_name) {
  auto const it = map_.find(simple_name);
  return it == map_.end() ? nullptr : it->second;
}

NamespaceMember* Namespace::FindMember(Token* simple_name) {
  return FindMember(simple_name->simple_name());
}

// NamespaceMember
Namespace* Namespace::ToNamespace() {
  return this;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
