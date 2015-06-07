// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/namespace.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Alias
//
Alias::Alias(NamespaceBody* namespace_body,
             Token* keyword,
             Token* name,
             Expression* reference)
    : SimpleNode(namespace_body, keyword, name) {
  DCHECK_EQ(keyword, TokenType::Using);
  set_child_at(0, reference);
}

Expression* Alias::reference() const {
  return child_at(0)->as<Expression>();
}

#if _DEBUG
// Node
bool Alias::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::NamespaceBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Import
//
Import::Import(NamespaceBody* namespace_body,
               Token* keyword,
               Expression* reference)
    : NamedNode(namespace_body, keyword, reference->token()),
      reference_(reference) {
  DCHECK_EQ(keyword, TokenType::Using);
}

#if _DEBUG
// Node
bool Import::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::NamespaceBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
NamespaceBody::NamespaceBody(Zone* zone,
                             NamespaceBody* outer,
                             Token* keyword,
                             Token* name)
    : ContainerNode(zone, outer, keyword, name), import_map_(zone) {
}

NamespaceBody* NamespaceBody::outer() const {
  return parent()->as<ast::NamespaceBody>();
}

void NamespaceBody::AddImport(Import* import) {
  import_map_[import->name()->atomic_string()] = import;
  AddMember(import);
}

Alias* NamespaceBody::FindAlias(Token* name) const {
  for (auto const member : members()) {
    auto const alias = member->as<ast::Alias>();
    if (alias && alias->name()->atomic_string() == name->atomic_string())
      return alias;
  }
  return nullptr;
}

Import* NamespaceBody::FindImport(Token* name) const {
  auto const it = import_map_.find(name->atomic_string());
  return it == import_map_.end() ? nullptr : it->second;
}

#if _DEBUG
// Node
bool NamespaceBody::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::NamespaceBody>();
}
#endif

}  // namespace ast
}  // namespace compiler
}  // namespace elang
