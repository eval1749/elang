// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "elang/compiler/ast/nodes.h"

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Node
//
Node::Node(ContainerNode* parent, Token* token)
    : parent_(parent), token_(token) {
}

Token* Node::name() const {
  return token();
}

#if _DEBUG
bool Node::CanBeMemberOf(ContainerNode* container) const {
  DCHECK(container);
  return false;
}
#endif

bool Node::IsDescendantOf(const Node* other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == other)
      return true;
  }
  return false;
}

// Visitable<Visitor>
void Node::Accept(Visitor* visitor) {
  __assume(visitor);
  NOTREACHED();
}

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
NamedNode::NamedNode(ContainerNode* parent, Token* keyword, Token* name)
    : Node(parent, keyword), name_(name) {
  DCHECK(name->is_name());
}

Token* NamedNode::name() const {
  return name_;
}

#if _DEBUG
bool NamedNode::CanBeNamedMemberOf(ContainerNode* container) const {
  DCHECK(container);
  return false;
}
#endif

base::string16 NamedNode::NewQualifiedName() const {
  std::vector<AtomicString*> ancestors;
  size_t length = 0u;
  for (auto runner = this; runner; runner = runner->parent()) {
    if (auto const ns_body = runner->as<ast::NamespaceBody>())
      runner = ns_body->owner();
    if (!runner->parent())
      break;
    auto const name_token = runner->name();
    DCHECK(name_token->has_atomic_string());
    if (!ancestors.empty())
      ++length;
    ancestors.push_back(name_token->atomic_string());
    length += ancestors.back()->string().length();
  }
  std::reverse(ancestors.begin(), ancestors.end());
  base::string16 buffer(length, '?');
  buffer.resize(0);
  auto first = true;
  for (auto const runner : ancestors) {
    if (first)
      first = false;
    else
      buffer.push_back('.');
    runner->string().AppendToString(&buffer);
  }
  return buffer;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
