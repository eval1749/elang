// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/node_factory.h"

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/import.h"
#include "elang/compiler/ast/local_variable.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

#define IMPLEMENT_ACCEPT(type) \
  void type::Accept(Visitor* visitor) { visitor->Visit##type(this); }
FOR_EACH_AST_NODE(IMPLEMENT_ACCEPT)
#undef IMPLEMENT_ACCEPT

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
NodeFactory::NodeFactory(Zone* zone) : zone_(zone) {
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
                             Expression* reference) {
  return new (zone_) Alias(namespace_body, keyword, alias_name, reference);
}

Class* NodeFactory::NewClass(NamespaceBody* namespace_body,
                             Modifiers modifiers,
                             Token* keyword,
                             Token* name) {
  return new (zone_) Class(zone_, namespace_body, modifiers, keyword, name);
}

Enum* NodeFactory::NewEnum(NamespaceBody* namespace_body,
                           Modifiers modifiers,
                           Token* keyword,
                           Token* name) {
  return new (zone_) Enum(zone_, namespace_body, modifiers, keyword, name);
}

EnumMember* NodeFactory::NewEnumMember(Enum* owner,
                                       Token* name,
                                       Expression* expression) {
  return new (zone_) EnumMember(owner, name, expression);
}

Field* NodeFactory::NewField(NamespaceBody* namespace_body,
                             Modifiers modifiers,
                             Expression* type,
                             Token* name,
                             Expression* expression) {
  return new (zone_) Field(namespace_body, modifiers, type, name, expression);
}

Import* NodeFactory::NewImport(NamespaceBody* namespace_body,
                               Token* keyword,
                               Expression* reference) {
  return new (zone_) Import(namespace_body, keyword, reference);
}

Method* NodeFactory::NewMethod(NamespaceBody* namespace_body,
                               MethodGroup* method_group,
                               Modifiers modifies,
                               Expression* type,
                               Token* name,
                               const std::vector<Token*>& type_parameters,
                               const std::vector<LocalVariable*>& parameters) {
  return new (zone_) Method(zone_, namespace_body, method_group, modifies, type,
                            name, type_parameters, parameters);
}

MethodGroup* NodeFactory::NewMethodGroup(NamespaceBody* namespace_body,
                                         Token* name) {
  DCHECK(namespace_body->owner()->is<Class>());
  DCHECK(name->is_name());
  return new (zone_) MethodGroup(zone_, namespace_body, name);
}

Namespace* NodeFactory::NewNamespace(NamespaceBody* namespace_body,
                                     Token* keyword,
                                     Token* name) {
  DCHECK_EQ(keyword->type(), TokenType::Namespace);
  return new (zone_) Namespace(zone_, namespace_body, keyword, name);
}

NamespaceBody* NodeFactory::NewNamespaceBody(NamespaceBody* outer,
                                             Namespace* owner) {
  return new (zone_) NamespaceBody(zone_, outer, owner);
}

//////////////////////////////////////////////////////////////////////
//
// Expression nodes
//
ArrayType* NodeFactory::NewArrayType(Token* op,
                                     Expression* element_type,
                                     const std::vector<int>& ranks) {
  return new (zone_) ArrayType(zone_, op, element_type, ranks);
}

Assignment* NodeFactory::NewAssignment(Token* op,
                                       Expression* left,
                                       Expression* right) {
  return new (zone_) Assignment(op, left, right);
}

BinaryOperation* NodeFactory::NewBinaryOperation(Token* op,
                                                 Expression* left,
                                                 Expression* right) {
  return new (zone_) BinaryOperation(op, left, right);
}

Conditional* NodeFactory::NewConditional(Token* op,
                                         Expression* cond_expr,
                                         Expression* then_expr,
                                         Expression* else_expr) {
  return new (zone_) Conditional(op, cond_expr, then_expr, else_expr);
}

ConstructedType* NodeFactory::NewConstructedType(
    Expression* blueprint_type,
    const std::vector<Expression*>& arguments) {
  return new (zone_) ConstructedType(zone_, blueprint_type, arguments);
}

InvalidExpression* NodeFactory::NewInvalidExpression(Token* token) {
  return new (zone_) InvalidExpression(token);
}

Literal* NodeFactory::NewLiteral(Token* literal) {
  return new (zone_) Literal(literal);
}

MemberAccess* NodeFactory::NewMemberAccess(
    Token* name,
    const std::vector<Expression*>& members) {
  return new (zone_) MemberAccess(zone_, name, members);
}

NameReference* NodeFactory::NewNameReference(Token* name) {
  return new (zone_) NameReference(name);
}

UnaryOperation* NodeFactory::NewUnaryOperation(Token* op, Expression* expr) {
  return new (zone_) UnaryOperation(op, expr);
}

//////////////////////////////////////////////////////////////////////
//
// Statement
//
BlockStatement* NodeFactory::NewBlockStatement(
    Token* keyword,
    const std::vector<Statement*> statements) {
  return new (zone_) BlockStatement(zone_, keyword, statements);
}

BreakStatement* NodeFactory::NewBreakStatement(Token* keyword) {
  return new (zone_) BreakStatement(keyword);
}

Call* NodeFactory::NewCall(Expression* callee,
                           const std::vector<Expression*> arguments) {
  return new (zone_) Call(zone_, callee, arguments);
}

CatchClause* NodeFactory::NewCatchClause(Token* keyword,
                                         Expression* type,
                                         LocalVariable* variable,
                                         BlockStatement* block) {
  return new (zone_) CatchClause(keyword, type, variable, block);
}

ContinueStatement* NodeFactory::NewContinueStatement(Token* keyword) {
  return new (zone_) ContinueStatement(keyword);
}

DoStatement* NodeFactory::NewDoStatement(Token* keyword,
                                         Statement* statement,
                                         Expression* condition) {
  return new (zone_) DoStatement(keyword, statement, condition);
}

EmptyStatement* NodeFactory::NewEmptyStatement(Token* keyword) {
  return new (zone_) EmptyStatement(keyword);
}

ExpressionList* NodeFactory::NewExpressionList(
    Token* keyword,
    const std::vector<Expression*>& expressions) {
  return new (zone_) ExpressionList(keyword, expressions);
}

ExpressionStatement* NodeFactory::NewExpressionStatement(
    Expression* expression) {
  return new (zone_) ExpressionStatement(expression);
}

ForEachStatement* NodeFactory::NewForEachStatement(Token* keyword,
                                                   LocalVariable* variable,
                                                   Expression* enumerable,
                                                   Statement* statement) {
  return new (zone_) ForEachStatement(keyword, variable, enumerable, statement);
}

ForStatement* NodeFactory::NewForStatement(Token* keyword,
                                           Statement* initializer,
                                           Expression* condition,
                                           Statement* step,
                                           Statement* statement) {
  return new (zone_)
      ForStatement(keyword, initializer, condition, step, statement);
}

IfStatement* NodeFactory::NewIfStatement(Token* keyword,
                                         Expression* condition,
                                         Statement* then_statement,
                                         Statement* else_statement) {
  return new (zone_)
      IfStatement(keyword, condition, then_statement, else_statement);
}

InvalidStatement* NodeFactory::NewInvalidStatement(Token* token) {
  return new (zone_) InvalidStatement(token);
}

LocalVariable* NodeFactory::NewLocalVariable(Token* keyword,
                                             Expression* type,
                                             Token* name,
                                             Expression* value) {
  return new (zone_) LocalVariable(keyword, type, name, value);
}

ReturnStatement* NodeFactory::NewReturnStatement(Token* keyword,
                                                 Expression* value) {
  return new (zone_) ReturnStatement(keyword, value);
}

ThrowStatement* NodeFactory::NewThrowStatement(Token* keyword,
                                               Expression* value) {
  return new (zone_) ThrowStatement(keyword, value);
}

TryStatement* NodeFactory::NewTryStatement(
    Token* keyword,
    BlockStatement* protected_block,
    const std::vector<CatchClause*>& catch_clauses,
    BlockStatement* finally_block) {
  return new (zone_) TryStatement(zone_, keyword, protected_block,
                                  catch_clauses, finally_block);
}

UsingStatement* NodeFactory::NewUsingStatement(Token* keyword,
                                               LocalVariable* variable,
                                               Expression* resource,
                                               Statement* statement) {
  return new (zone_) UsingStatement(keyword, variable, resource, statement);
}

VarStatement* NodeFactory::NewVarStatement(
    Token* keyword,
    const std::vector<ast::LocalVariable*>& variables) {
  return new (zone_) VarStatement(zone_, keyword, variables);
}

WhileStatement* NodeFactory::NewWhileStatement(Token* keyword,
                                               Expression* condition,
                                               Statement* statement) {
  return new (zone_) WhileStatement(keyword, condition, statement);
}

YieldStatement* NodeFactory::NewYieldStatement(Token* keyword,
                                               Expression* value) {
  return new (zone_) YieldStatement(keyword, value);
}

//////////////////////////////////////////////////////////////////////
//
// Utility
//
Node* NodeFactory::RememberNode(Node* node) {
  return node;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
