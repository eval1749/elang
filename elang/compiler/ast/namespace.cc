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
Namespace::Namespace(NamespaceBody* namespace_body, const Token& keyword,
                     const Token& simple_name)
    : NamespaceMember(namespace_body, keyword, simple_name) {
}

Namespace::~Namespace() {
}

void Namespace::AddMember(NamespaceMember* member) {
  DCHECK(!member->is<Alias>());
  // We keep first member declaration.
  if (FindMember(member->simple_name()))
    return;
  map_[member->simple_name().id()] = member;
}

void Namespace::AddNamespaceBody(NamespaceBody* namespace_body) {
  bodies_.push_back(namespace_body);
}

NamespaceMember* Namespace::FindMember(const Token& simple_name) {
  auto const it = map_.find(simple_name.id());
  return it == map_.end() ? nullptr : it->second;
}

// NamespaceMember
Namespace* Namespace::ToNamespace() {
  return this;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
