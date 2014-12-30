// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace_body.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/import.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
NamespaceBody::NamespaceBody(NamespaceBody* outer, Namespace* owner)
    : outer_(outer), owner_(owner) {
  if (outer_) {
    DCHECK_EQ(outer_->owner(), owner->outer());
  } else {
    DCHECK(!owner->outer());
    DCHECK_EQ(owner->token(), TokenType::Namespace);
  }
}

NamespaceBody::~NamespaceBody() {
}

const std::vector<Alias*>& NamespaceBody::aliases() const {
  DCHECK(owner_->ToNamespace());
  return aliases_;
}

const std::vector<Import*>& NamespaceBody::imports() const {
  DCHECK(owner_->ToNamespace());
  return imports_;
}

void NamespaceBody::AddAlias(Alias* alias) {
  DCHECK(owner_->ToNamespace());
  aliases_.push_back(alias);
  alias_map_[alias->name()->simple_name()] = alias;
  members_.push_back(alias);
}

void NamespaceBody::AddImport(Import* import) {
  DCHECK(owner_->ToNamespace());
  imports_.push_back(import);
  import_map_[import->name()->simple_name()] = import;
  members_.push_back(import);
}

void NamespaceBody::AddMember(NamespaceMember* member) {
  DCHECK(!member->as<Alias>());
  owner()->AddMember(member);
  members_.push_back(member);
}

Alias* NamespaceBody::FindAlias(Token* name) {
  auto const it = alias_map_.find(name->simple_name());
  return it == alias_map_.end() ? nullptr : it->second;
}

Import* NamespaceBody::FindImport(Token* name) {
  auto const it = import_map_.find(name->simple_name());
  return it == import_map_.end() ? nullptr : it->second;
}

NamespaceMember* NamespaceBody::FindMember(Token* name) {
  return owner_->FindMember(name);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
