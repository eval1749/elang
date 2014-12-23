// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node_factory.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/qualified_name.h"

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
                                     QualifiedName&& name) {
  auto const node = new Namespace(outer, keyword, std::move(name));
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

}   // namespace ast
}  // namespace compiler
}  // namespace elang
