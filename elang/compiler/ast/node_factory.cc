// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node_factory.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
NodeFactory::NodeFactory() {
}

NodeFactory::~NodeFactory() {
}

Alias* NodeFactory::NewAlias(Namespace* outer, const Token& keyword,
                             const Token& simple_name,
                             const QualifiedName& target_name) {
  auto const node = new Alias(outer, keyword, simple_name, target_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Class* NodeFactory::NewClass(Namespace* outer, const Token& keyword,
                             const Token& simple_name) {
  auto const node = new Class(outer, keyword, simple_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Enum* NodeFactory::NewEnum(Namespace* outer, const Token& keyword,
                           const Token& simple_name) {
  auto const node = new Enum(outer, keyword, simple_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

EnumMember* NodeFactory::NewEnumMember(Enum* owner, const Token& simple_name,
                                       Expression* expression) {
  auto const node = new EnumMember(owner, simple_name, expression);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Namespace* NodeFactory::NewNamespace(Namespace* outer, const Token& keyword,
                                     const Token& simple_name) {
  DCHECK_EQ(keyword.type(), TokenType::Namespace);
  auto const node = new Namespace(outer, keyword, simple_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

void NodeFactory::RemoveAll() {
  nodes_.clear();
}

}   // namespace ast
}  // namespace compiler
}  // namespace elang
