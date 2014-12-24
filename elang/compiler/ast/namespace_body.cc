// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace_body.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
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
    Token keyword;
    QualifiedName name;
    ImportDef(const Token& keyword, const QualifiedName& real_name);
};

NamespaceBody::ImportDef::ImportDef(const Token& keyword,
                                    const QualifiedName& name)
    : keyword(keyword), name(name) {
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
NamespaceBody::NamespaceBody(NamespaceBody* outer, Namespace* owner,
                             SourceCode* source_code)
    : outer_(outer), owner_(owner), source_code_(source_code) {
}

NamespaceBody::~NamespaceBody() {
  for (auto const import_def : import_defs_)
    delete import_def;
}

void NamespaceBody::AddImport(const Token& keyword, const QualifiedName& name) {
  DCHECK_EQ(keyword.type(), TokenType::Using);
  import_defs_.push_back(new ImportDef(keyword, name));
}

void NamespaceBody::AddMember(NamespaceMember* member) {
  if (auto const alias = member->as<Alias>()) {
    aliases_.push_back(alias);
    alias_map_[alias->simple_name().id()] = alias;
  }
  members_.push_back(member);
}

Alias* NamespaceBody::FindAlias(const Token& simple_name) {
  auto const it = alias_map_.find(simple_name.id());
  return it == alias_map_.end() ? nullptr : it->second;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
