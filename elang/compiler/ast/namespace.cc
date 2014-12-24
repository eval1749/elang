// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace.h"

#include "base/logging.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Namespace::ImportDef
//
struct Namespace::ImportDef {
    Token keyword;
    QualifiedName name;
    ImportDef(const Token& keyword, const QualifiedName& real_name);
};

Namespace::ImportDef::ImportDef(const Token& keyword,
                                const QualifiedName& name)
    : keyword(keyword), name(name) {
}

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
Namespace::Namespace(Namespace* outer, const Token& keyword,
                     const Token& simple_name)
    : NamespaceMember(outer, keyword, simple_name),
      name_(QualifiedName(simple_name)) {
  DCHECK_EQ(keyword.type(), TokenType::Namespace);
}

Namespace::Namespace(Namespace* outer, const Token& keyword,
                     QualifiedName&& name)
    : NamespaceMember(outer, keyword, name.simple_name()),
      name_(std::move(name)) {
  DCHECK_EQ(keyword.type(), TokenType::Namespace);
}

Namespace::~Namespace() {
  for (auto const import_def : import_defs_)
    delete import_def;
}

void Namespace::AddImport(const Token& keyword, const QualifiedName& name) {
  DCHECK_EQ(keyword.type(), TokenType::Using);
  import_defs_.push_back(new ImportDef(keyword, name));
}

void Namespace::AddMember(NamespaceMember* member) {
  if (!FindMember(member->simple_name()))
    map_[member->simple_name().id()] = member;
  members_.push_back(member);
}

NamespaceMember* Namespace::FindMember(const Token& simple_name) {
  auto const it = map_.find(simple_name.id());
  return it == map_.end() ? nullptr : it->second;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
