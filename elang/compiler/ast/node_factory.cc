// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node_factory.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/array_type.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/block_statement.h"
#include "elang/compiler/ast/break_statement.h"
#include "elang/compiler/ast/catch_clause.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/constructed_type.h"
#include "elang/compiler/ast/continue_statement.h"
#include "elang/compiler/ast/do_statement.h"
#include "elang/compiler/ast/empty_statement.h"
#include "elang/compiler/ast/expression_statement.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/if_statement.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/local_variable.h"
#include "elang/compiler/ast/member_access.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/method_group.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/return_statement.h"
#include "elang/compiler/ast/throw_statement.h"
#include "elang/compiler/ast/try_statement.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/ast/using_statement.h"
#include "elang/compiler/ast/var_statement.h"
#include "elang/compiler/ast/while_statement.h"
#include "elang/compiler/ast/yield_statement.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/modifiers.h"
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
                             Token* keyword,
                             Token* alias_name,
                             const QualifiedName& target_name) {
  auto const node = new Alias(namespace_body, keyword, alias_name, target_name);
  RememberNode(node);
  return node;
}

Class* NodeFactory::NewClass(NamespaceBody* namespace_body,
                             Modifiers modifiers,
                             Token* keyword,
                             Token* name) {
  auto const node = new Class(namespace_body, modifiers, keyword, name);
  RememberNode(node);
  return node;
}

Enum* NodeFactory::NewEnum(NamespaceBody* namespace_body,
                           Modifiers modifiers,
                           Token* keyword,
                           Token* name) {
  auto const node = new Enum(namespace_body, modifiers, keyword, name);
  RememberNode(node);
  return node;
}

EnumMember* NodeFactory::NewEnumMember(Enum* owner,
                                       Token* name,
                                       Expression* expression) {
  auto const node = new EnumMember(owner, name, expression);
  RememberNode(node);
  return node;
}

Field* NodeFactory::NewField(NamespaceBody* namespace_body,
                             Modifiers modifiers,
                             Expression* type,
                             Token* name,
                             Expression* expression) {
  auto const node =
      new Field(namespace_body, modifiers, type, name, expression);
  RememberNode(node);
  return node;
}

Method* NodeFactory::NewMethod(NamespaceBody* namespace_body,
                               MethodGroup* method_group,
                               Modifiers modifies,
                               Expression* type,
                               Token* name,
                               const std::vector<Token*>& type_parameters,
                               const std::vector<LocalVariable*>& parameters) {
  auto const node = new Method(namespace_body, method_group, modifies, type,
                               name, type_parameters, parameters);
  RememberNode(node);
  return node;
}

MethodGroup* NodeFactory::NewMethodGroup(NamespaceBody* namespace_body,
                                         Token* name) {
  DCHECK(namespace_body->owner()->is<Class>());
  DCHECK(name->is_name());
  auto const node = new MethodGroup(namespace_body, name);
  RememberNode(node);
  return node;
}

Namespace* NodeFactory::NewNamespace(NamespaceBody* namespace_body,
                                     Token* keyword,
                                     Token* name) {
  DCHECK_EQ(keyword->type(), TokenType::Namespace);
  auto const node = new Namespace(namespace_body, keyword, name);
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

Assignment* NodeFactory::NewAssignment(Token* op,
                                       Expression* left,
                                       Expression* right) {
  auto const node = new Assignment(op, left, right);
  RememberNode(node);
  return node;
}

BinaryOperation* NodeFactory::NewBinaryOperation(Token* op,
                                                 Expression* left,
                                                 Expression* right) {
  auto const node = new BinaryOperation(op, left, right);
  RememberNode(node);
  return node;
}

Conditional* NodeFactory::NewConditional(Token* op,
                                         Expression* cond_expr,
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
// Statement
//
BlockStatement* NodeFactory::NewBlockStatement(
    Token* keyword,
    const std::vector<Statement*> statements) {
  auto const node = new BlockStatement(keyword, statements);
  RememberNode(node);
  return node;
}

BreakStatement* NodeFactory::NewBreakStatement(Token* keyword) {
  auto const node = new BreakStatement(keyword);
  RememberNode(node);
  return node;
}

CatchClause* NodeFactory::NewCatchClause(Token* keyword,
                                         Expression* type,
                                         LocalVariable* variable,
                                         BlockStatement* block) {
  auto const node = new CatchClause(keyword, type, variable, block);
  RememberNode(node);
  return node;
}

ContinueStatement* NodeFactory::NewContinueStatement(Token* keyword) {
  auto const node = new ContinueStatement(keyword);
  RememberNode(node);
  return node;
}

DoStatement* NodeFactory::NewDoStatement(Token* keyword,
                                         Statement* statement,
                                         Expression* condition) {
  auto const node = new DoStatement(keyword, statement, condition);
  RememberNode(node);
  return node;
}

EmptyStatement* NodeFactory::NewEmptyStatement(Token* keyword) {
  auto const node = new EmptyStatement(keyword);
  RememberNode(node);
  return node;
}

ExpressionStatement* NodeFactory::NewExpressionStatement(
    Expression* expression) {
  auto const node = new ExpressionStatement(expression);
  RememberNode(node);
  return node;
}

IfStatement* NodeFactory::NewIfStatement(Token* keyword,
                                         Expression* condition,
                                         Statement* then_statement,
                                         Statement* else_statement) {
  auto const node =
      new IfStatement(keyword, condition, then_statement, else_statement);
  RememberNode(node);
  return node;
}

LocalVariable* NodeFactory::NewLocalVariable(Token* keyword,
                                             Expression* type,
                                             Token* name,
                                             Expression* value) {
  auto const node = new LocalVariable(keyword, type, name, value);
  RememberNode(node);
  return node;
}

ReturnStatement* NodeFactory::NewReturnStatement(Token* keyword,
                                                 Expression* value) {
  auto const node = new ReturnStatement(keyword, value);
  RememberNode(node);
  return node;
}

ThrowStatement* NodeFactory::NewThrowStatement(Token* keyword,
                                               Expression* value) {
  auto const node = new ThrowStatement(keyword, value);
  RememberNode(node);
  return node;
}

TryStatement* NodeFactory::NewTryStatement(
    Token* keyword,
    BlockStatement* protected_block,
    const std::vector<CatchClause*>& catch_clauses,
    BlockStatement* finally_block) {
  auto const node =
      new TryStatement(keyword, protected_block, catch_clauses, finally_block);
  RememberNode(node);
  return node;
}

UsingStatement* NodeFactory::NewUsingStatement(Token* keyword,
                                               LocalVariable* variable,
                                               Expression* resource,
                                               Statement* statement) {
  auto const node = new UsingStatement(keyword, variable, resource, statement);
  RememberNode(node);
  return node;
}

VarStatement* NodeFactory::NewVarStatement(
    Token* keyword,
    const std::vector<ast::LocalVariable*>& variables) {
  auto const node = new VarStatement(keyword, variables);
  RememberNode(node);
  return node;
}

WhileStatement* NodeFactory::NewWhileStatement(Token* keyword,
                                               Expression* condition,
                                               Statement* statement) {
  auto const node = new WhileStatement(keyword, condition, statement);
  RememberNode(node);
  return node;
}

YieldStatement* NodeFactory::NewYieldStatement(Token* keyword,
                                               Expression* value) {
  auto const node = new YieldStatement(keyword, value);
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

}  // namespace ast
}  // namespace compiler
}  // namespace elang
