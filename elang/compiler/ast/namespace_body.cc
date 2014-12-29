// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace_body.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody::ImportDef
//
struct NamespaceBody::ImportDef {
  Token* keyword;
  QualifiedName name;
  ImportDef(Token* keyword, const QualifiedName& real_name);
};

NamespaceBody::ImportDef::ImportDef(Token* keyword, const QualifiedName& name)
    : keyword(keyword), name(name) {
}

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
  for (auto const import_def : import_defs_)
    delete import_def;
}

const std::vector<Alias*>& NamespaceBody::aliases() const {
  DCHECK(owner_->ToNamespace());
  return aliases_;
}

void NamespaceBody::AddImport(Token* keyword, const QualifiedName& name) {
  DCHECK(owner_->ToNamespace());
  DCHECK_EQ(keyword->type(), TokenType::Using);
  import_defs_.push_back(new ImportDef(keyword, name));
}

void NamespaceBody::AddAlias(Alias* alias) {
  DCHECK(owner_->ToNamespace());
  aliases_.push_back(alias);
  alias_map_[alias->name()->simple_name()] = alias;
  members_.push_back(alias);
}

void NamespaceBody::AddMember(NamespaceMember* member) {
  DCHECK(!member->as<Alias>());
  owner()->AddMember(member);
  members_.push_back(member);
}

Alias* NamespaceBody::FindAlias(Token* simple_name) {
  DCHECK(owner_->ToNamespace());
  auto const it = alias_map_.find(simple_name->simple_name());
  return it == alias_map_.end() ? nullptr : it->second;
}

NamespaceMember* NamespaceBody::FindMember(Token* simple_name) {
  return owner_->FindMember(simple_name);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
