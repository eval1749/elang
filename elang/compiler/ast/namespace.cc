// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/import.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// MemberContainer
//
MemberContainer::MemberContainer(Zone* zone,
                                 NamespaceBody* namespace_body,
                                 Token* keyword,
                                 Token* name)
    : NamespaceMember(namespace_body, keyword, name),
      bodies_(zone),
      map_(zone) {
}

void MemberContainer::AcceptForMembers(Visitor* visitor) {
  for (auto const body : bodies_) {
    for (auto const member : body->members())
      member->Accept(visitor);
  }
}

void MemberContainer::AddMember(NamedNode* member) {
  DCHECK_EQ(this, member->parent());
  DCHECK(!member->is<Alias>() && !member->is<Import>() &&
         !member->is<Method>());
  // We keep first member declaration.
  if (FindMember(member->name()))
    return;
  map_[member->name()->simple_name()] = member;
}

void MemberContainer::AddNamespaceBody(NamespaceBody* namespace_body) {
  bodies_.push_back(namespace_body);
}

NamedNode* MemberContainer::FindMember(AtomicString* name) {
  auto const it = map_.find(name);
  return it == map_.end() ? nullptr : it->second;
}

NamedNode* MemberContainer::FindMember(Token* name) {
  return FindMember(name->simple_name());
}

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
Namespace::Namespace(Zone* zone,
                     NamespaceBody* namespace_body,
                     Token* keyword,
                     Token* name)
    : MemberContainer(zone,
                      namespace_body,
                      keyword,
                      name) {
  DCHECK_EQ(keyword, TokenType::Namespace);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
