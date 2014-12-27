// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node_factory.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/array_type.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/constructed_type.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/member_access.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

#define IMPLEMENT_ACCEPT(type) \
  void type::Accept(Visitor* visitor) { visitor->Visit##type(this); }
AST_NODE_LIST(IMPLEMENT_ACCEPT)
#undef IMPLEMENT_ACCEPT

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
NodeFactory::NodeFactory() {
}

NodeFactory::~NodeFactory() {
}

//////////////////////////////////////////////////////////////////////
//
// Declaration related nodes
//
Alias* NodeFactory::NewAlias(NamespaceBody* namespace_body,
                             Token* keyword, Token* simple_name,
                             const QualifiedName& target_name) {
  auto const node = new Alias(namespace_body, keyword, simple_name,
                              target_name);
  RememberNode(node);
  return node;
}

Class* NodeFactory::NewClass(NamespaceBody* namespace_body,
                             Token* keyword, Token* simple_name) {
  auto const node = new Class(namespace_body, keyword, simple_name);
  RememberNode(node);
  return node;
}

Enum* NodeFactory::NewEnum(NamespaceBody* namespace_body,
                           Token* keyword, Token* simple_name) {
  auto const node = new Enum(namespace_body, keyword, simple_name);
  RememberNode(node);
  return node;
}

EnumMember* NodeFactory::NewEnumMember(Enum* owner, Token* simple_name,
                                       Expression* expression) {
  auto const node = new EnumMember(owner, simple_name, expression);
  RememberNode(node);
  return node;
}

Field* NodeFactory::NewField(NamespaceBody* namespace_body, Expression* type,
                             Token* name, Expression* expression) {
  auto const node = new Field(namespace_body, type, name, expression);
  RememberNode(node);
  return node;
}

Namespace* NodeFactory::NewNamespace(NamespaceBody* namespace_body,
                                     Token* keyword,
                                     Token* simple_name) {
  DCHECK_EQ(keyword->type(), TokenType::Namespace);
  auto const node = new Namespace(namespace_body, keyword, simple_name);
  RememberNode(node);
  return node;
}

//////////////////////////////////////////////////////////////////////
//
// Expression nodes
//
ArrayType* NodeFactory::NewArrayType(Token* op,
                                     Expression* element_type,
                                     const std::vector<int>& ranks) {
  auto const node = new ArrayType(op, element_type, ranks);
  RememberNode(node);
  return node;
}

Assignment* NodeFactory::NewAssignment(Token* op, Expression* left,
                                       Expression* right) {
  auto const node = new Assignment(op, left, right);
  RememberNode(node);
  return node;
}

BinaryOperation* NodeFactory::NewBinaryOperation(Token* op, Expression* left,
                                                 Expression* right) {
  auto const node = new BinaryOperation(op, left, right);
  RememberNode(node);
  return node;
}

Conditional* NodeFactory::NewConditional(Token* op, Expression* cond_expr,
                                         Expression* then_expr,
                                         Expression* else_expr) {
  auto const node = new Conditional(op, cond_expr, then_expr, else_expr);
  RememberNode(node);
  return node;
}

ConstructedType* NodeFactory::NewConstructedType(
    Token* op,
    Expression* blueprint_type,
    const std::vector<Expression*>& arguments) {
  auto const node = new ConstructedType(op, blueprint_type, arguments);
  RememberNode(node);
  return node;
}

Literal* NodeFactory::NewLiteral(Token* literal) {
  auto const node = new Literal(literal);
  RememberNode(node);
  return node;
}

MemberAccess* NodeFactory::NewMemberAccess(
    const std::vector<Expression*>& members) {
  auto const node = new MemberAccess(members);
  RememberNode(node);
  return node;
}

NameReference* NodeFactory::NewNameReference(Token* name) {
  auto const node = new NameReference(name);
  RememberNode(node);
  return node;
}

UnaryOperation* NodeFactory::NewUnaryOperation(Token* op, Expression* expr) {
  auto const node = new UnaryOperation(op, expr);
  RememberNode(node);
  return node;
}

//////////////////////////////////////////////////////////////////////
//
// Utility
//
Node* NodeFactory::RememberNode(Node* node) {
  nodes_.push_back(std::unique_ptr<Node>(node));
  return node;
}

void NodeFactory::RemoveAll() {
  nodes_.clear();
}

}   // namespace ast
}  // namespace compiler
}  // namespace elang
