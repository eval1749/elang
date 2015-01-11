// Copyright 2014 Project Vogue. All rights reserved.
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

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
Namespace::Namespace(Zone* zone, Namespace* parent, Token* keyword, Token* name)
    : ContainerNode(zone, parent, keyword, name) {
  DCHECK_EQ(keyword, TokenType::Namespace);
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
NamespaceBody::NamespaceBody(Zone* zone, NamespaceBody* outer, Namespace* owner)
    : ContainerNode(zone, outer, owner->name(), owner->name()),
      imports_(zone),
      loaded_(false),
      namespace_(owner) {
}

NamespaceBody* NamespaceBody::outer() const {
  return parent() ? parent()->as<ast::NamespaceBody>() : nullptr;
}

void NamespaceBody::AddMember(Node* member) {
  DCHECK(member->CanBeInNamespaceBody());
  ContainerNode::AddMember(member);
}

void NamespaceBody::AddNamedMember(NamedNode* member) {
  if (auto const import = member->as<ast::Import>())
    imports_.push_back(import);
  ContainerNode::AddNamedMember(member);
  namespace_->AddNamedMember(member);
}

// Node
bool NamespaceBody::CanBeInNamespaceBody() const {
  return true;
}

// ContainerNode
NamedNode* NamespaceBody::FindMemberMore(AtomicString* name) {
  return namespace_->FindMember(name);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
