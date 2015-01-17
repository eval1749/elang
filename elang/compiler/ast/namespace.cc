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
    : NamedNode(namespace_body, keyword, name), reference_(reference) {
  DCHECK_EQ(keyword, TokenType::Using);
}

#if _DEBUG
// Node
bool Alias::CanBeMemberOf(ContainerNode* container) const {
  return container->is<ast::NamespaceBody>();
}

// NamedNode
bool Alias::CanBeNamedMemberOf(ContainerNode* container) const {
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
// Namespace
//
Namespace::Namespace(Zone* zone, Namespace* parent, Token* keyword, Token* name)
    : NamespaceNode(zone, parent, keyword, name) {
  DCHECK_EQ(keyword, TokenType::Namespace);
}

#if _DEBUG
// Node
bool Namespace::CanBeMemberOf(ContainerNode* container) const {
  DCHECK(container);
  return false;
}

// NamedNode
bool Namespace::CanBeNamedMemberOf(ContainerNode* container) const {
  return container->is<ast::Namespace>() || container->is<ast::NamespaceBody>();
}
#endif

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
NamespaceBody::NamespaceBody(Zone* zone, NamespaceBody* outer, Namespace* owner)
    : BodyNode(zone, outer, owner), import_map_(zone), loaded_(false) {
}

NamespaceBody* NamespaceBody::outer() const {
  return parent()->as<ast::NamespaceBody>();
}

Namespace* NamespaceBody::owner() const {
  return BodyNode::owner()->as<ast::Namespace>();
}

void NamespaceBody::AddImport(Import* import) {
  import_map_[import->name()->atomic_string()] = import;
  AddMember(import);
}

Alias* NamespaceBody::FindAlias(Token* name) const {
  auto const present = FindMember(name);
  return present ? present->as<ast::Alias>() : nullptr;
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
