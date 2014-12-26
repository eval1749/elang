// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node_factory.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/expression.h"
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

Alias* NodeFactory::NewAlias(NamespaceBody* namespace_body,
                             Token* keyword, Token* simple_name,
                             const QualifiedName& target_name) {
  auto const node = new Alias(namespace_body, keyword, simple_name,
                              target_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Class* NodeFactory::NewClass(NamespaceBody* namespace_body,
                             Token* keyword, Token* simple_name) {
  auto const node = new Class(namespace_body, keyword, simple_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Enum* NodeFactory::NewEnum(NamespaceBody* namespace_body,
                           Token* keyword, Token* simple_name) {
  auto const node = new Enum(namespace_body, keyword, simple_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

EnumMember* NodeFactory::NewEnumMember(Enum* owner, Token* simple_name,
                                       Expression* expression) {
  auto const node = new EnumMember(owner, simple_name, expression);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Expression* NodeFactory::NewExpression(Token* operator_token,
                                       std::vector<Expression*> operands) {
  auto const node = new Expression(operator_token, operands);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Expression* NodeFactory::NewExpression(Token* operator_token,
                                    Expression* operand0,
                                    Expression* operand1) {
  auto const node = new Expression(operator_token, operand0, operand1);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Expression* NodeFactory::NewExpression(Token* operator_token,
                                    Expression* operand0) {
  auto const node = new Expression(operator_token, operand0);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Expression* NodeFactory::NewExpression(Token* operator_token) {
  auto const node = new Expression(operator_token);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

Namespace* NodeFactory::NewNamespace(NamespaceBody* namespace_body,
                                     Token* keyword,
                                     Token* simple_name) {
  DCHECK_EQ(keyword->type(), TokenType::Namespace);
  auto const node = new Namespace(namespace_body, keyword, simple_name);
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

void NodeFactory::RemoveAll() {
  nodes_.clear();
}

}   // namespace ast
}  // namespace compiler
}  // namespace elang
