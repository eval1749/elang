// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/factory.h"

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/token_factory.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

namespace {

Namespace* NewGlobalNamespace(CompilationSession* session, Factory* factory) {
  auto const keyword = session->token_factory()->NewSystemKeyword(
      TokenType::Namespace, L"namespace");
  auto const name = session->token_factory()->NewSystemName(L"global");
  return factory->NewNamespace(nullptr, keyword, name);
}

}  // namespace

// Visitor related functions.
#define V(Name) \
  void Name::Accept(Visitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

// Default implementations do nothing. Each derived visitor class implements
// override for interested classes.
#define V(type) \
  void Visitor::Visit##type(type* node) { DoDefaultVisit(node); }
FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

void Visitor::DoDefaultVisit(Node* node) {
  auto const container = node->as<ContainerNode>();
  if (!container)
    return;
  container->AcceptForMembers(this);
}

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory(CompilationSession* session)
    : ZoneUser(session->zone()),
      global_namespace_(NewGlobalNamespace(session, this)) {
}

Factory::~Factory() {
}

//////////////////////////////////////////////////////////////////////
//
// Declaration related nodes
//
Alias* Factory::NewAlias(NamespaceBody* namespace_body,
                         Token* keyword,
                         Token* alias_name,
                         Expression* reference) {
  auto const node =
      new (zone()) Alias(namespace_body, keyword, alias_name, reference);
  SetParent(reference, node);
  return node;
}

Class* Factory::NewClass(NamespaceNode* outer,
                         Modifiers modifiers,
                         Token* keyword,
                         Token* name) {
  return new (zone()) Class(zone(), outer, modifiers, keyword, name);
}

ClassBody* Factory::NewClassBody(BodyNode* outer,
                                 Modifiers modifiers,
                                 Token* keyword,
                                 Token* name,
                                 const std::vector<Type*>& base_class_names) {
  auto const node = new (zone())
      ClassBody(zone(), outer, modifiers, keyword, name, base_class_names);
  for (auto const base_class_name : base_class_names)
    SetParent(base_class_name, node);
  return node;
}

Const* Factory::NewConst(ClassBody* outer,
                         Modifiers modifiers,
                         Token* keyword,
                         Type* type,
                         Token* name,
                         Expression* expression) {
  auto const node =
      new (zone()) Const(outer, modifiers, keyword, type, name, expression);
  SetParent(expression, node);
  if (!type->parent_)
    SetParent(type, outer);
  return node;
}

Enum* Factory::NewEnum(BodyNode* container,
                       Modifiers modifiers,
                       Token* keyword,
                       Token* name,
                       Type* enum_base) {
  auto const node =
      new (zone()) Enum(zone(), container, modifiers, keyword, name, enum_base);
  if (enum_base)
    SetParent(enum_base, node);
  return node;
}

EnumMember* Factory::NewEnumMember(Enum* owner,
                                   Token* name,
                                   Expression* explicit_expression,
                                   Expression* implicit_expression) {
  if (explicit_expression)
    DCHECK(!implicit_expression);
  else
    DCHECK(implicit_expression);
  auto const node = new (zone())
      EnumMember(owner, name, explicit_expression, implicit_expression);
  if (explicit_expression)
    SetParent(explicit_expression, node);
  if (implicit_expression)
    SetParent(implicit_expression, node);
  return node;
}

Field* Factory::NewField(ClassBody* outer,
                         Modifiers modifiers,
                         Token* keyword,
                         Type* type,
                         Token* name,
                         Expression* expression) {
  auto const node =
      new (zone()) Field(outer, modifiers, keyword, type, name, expression);
  SetParent(expression, node);
  if (!type->parent_)
    SetParent(type, outer);
  return node;
}

Import* Factory::NewImport(NamespaceBody* namespace_body,
                           Token* keyword,
                           Expression* reference) {
  auto const node = new (zone()) Import(namespace_body, keyword, reference);
  SetParent(reference, node);
  return node;
}

Method* Factory::NewMethod(ClassBody* outer,
                           Modifiers modifies,
                           Type* return_type,
                           Token* name,
                           const std::vector<Token*>& type_parameters) {
  auto const node = new (zone())
      Method(zone(), outer, modifies, return_type, name, type_parameters);
  SetParent(return_type, node);
  return node;
}

MethodBody* Factory::NewMethodBody(Method* method) {
  return new (zone()) MethodBody(zone(), method);
}

Namespace* Factory::NewNamespace(Namespace* outer,
                                 Token* keyword,
                                 Token* name) {
  DCHECK_EQ(keyword->type(), TokenType::Namespace);
  return new (zone()) Namespace(zone(), outer, keyword, name);
}

NamespaceBody* Factory::NewNamespaceBody(NamespaceBody* outer,
                                         Namespace* owner) {
  return new (zone()) NamespaceBody(zone(), outer, owner);
}

//////////////////////////////////////////////////////////////////////
//
// Expression nodes
//
ArrayAccess* Factory::NewArrayAccess(Token* bracket,
                                     Expression* array,
                                     const std::vector<Expression*> indexes) {
  auto const node = new (zone()) ArrayAccess(zone(), bracket, array, indexes);
  SetParent(array, node);
  for (auto const index : indexes)
    SetParent(index, node);
  return node;
}

Assignment* Factory::NewAssignment(Token* op,
                                   Expression* left,
                                   Expression* right) {
  auto const node = new (zone()) Assignment(op, left, right);
  SetParent(left, node);
  SetParent(right, node);
  return node;
}

BinaryOperation* Factory::NewBinaryOperation(Token* op,
                                             Expression* left,
                                             Expression* right) {
  auto const node = new (zone()) BinaryOperation(op, left, right);
  SetParent(left, node);
  SetParent(right, node);
  return node;
}

Conditional* Factory::NewConditional(Token* op,
                                     Expression* cond_expr,
                                     Expression* then_expr,
                                     Expression* else_expr) {
  auto const node =
      new (zone()) Conditional(op, cond_expr, then_expr, else_expr);
  SetParent(cond_expr, node);
  SetParent(then_expr, node);
  SetParent(else_expr, node);
  return node;
}

ConstructedName* Factory::NewConstructedName(
    Expression* reference,
    const std::vector<Type*>& arguments) {
  auto const node = new (zone()) ConstructedName(zone(), reference, arguments);
  SetParent(reference, node);
  for (auto const argument : arguments)
    SetParent(argument, node);
  return node;
}

IncrementExpression* Factory::NewIncrementExpression(Token* op,
                                                     Expression* expr) {
  auto const node = new (zone()) IncrementExpression(op, expr);
  SetParent(expr, node);
  return node;
}

InvalidExpression* Factory::NewInvalidExpression(Token* token) {
  return new (zone()) InvalidExpression(token);
}

Literal* Factory::NewLiteral(Token* literal) {
  return new (zone()) Literal(literal);
}

MemberAccess* Factory::NewMemberAccess(Expression* container, Token* member) {
  auto const node = new (zone()) MemberAccess(container, member);
  SetParent(container, node);
  return node;
}

NameReference* Factory::NewNameReference(Token* name) {
  return new (zone()) NameReference(name);
}

Expression* Factory::NewNoExpression(Token* token) {
  return new (zone()) NoExpression(token);
}

ParameterReference* Factory::NewParameterReference(Token* name,
                                                   Parameter* parameter) {
  return new (zone()) ParameterReference(name, parameter);
}

UnaryOperation* Factory::NewUnaryOperation(Token* op, Expression* expr) {
  auto const node = new (zone()) UnaryOperation(op, expr);
  SetParent(expr, node);
  return node;
}

VariableReference* Factory::NewVariableReference(Token* name,
                                                 Variable* variable) {
  return new (zone()) VariableReference(name, variable);
}

//////////////////////////////////////////////////////////////////////
//
// Statement
//
BlockStatement* Factory::NewBlockStatement(
    Token* keyword,
    const std::vector<Statement*> statements) {
  auto const node = new (zone()) BlockStatement(zone(), keyword, statements);
  for (auto const statement : statements)
    SetParent(statement, node);
  return node;
}

BreakStatement* Factory::NewBreakStatement(Token* keyword) {
  return new (zone()) BreakStatement(keyword);
}

Call* Factory::NewCall(Expression* callee,
                       const std::vector<Expression*> arguments) {
  auto const node = new (zone()) Call(zone(), callee, arguments);
  SetParent(callee, node);
  for (auto const argument : arguments)
    SetParent(argument, node);
  return node;
}

CatchClause* Factory::NewCatchClause(Token* keyword,
                                     Type* type,
                                     Variable* variable,
                                     BlockStatement* block) {
  auto const node = new (zone()) CatchClause(keyword, type, variable, block);
  SetParent(type, node);
  SetParent(block, node);
  return node;
}

ContinueStatement* Factory::NewContinueStatement(Token* keyword) {
  return new (zone()) ContinueStatement(keyword);
}

DoStatement* Factory::NewDoStatement(Token* keyword,
                                     Statement* statement,
                                     Expression* condition) {
  auto const node = new (zone()) DoStatement(keyword, statement, condition);
  SetParent(statement, node);
  SetParent(condition, node);
  return node;
}

EmptyStatement* Factory::NewEmptyStatement(Token* keyword) {
  return new (zone()) EmptyStatement(keyword);
}

ExpressionList* Factory::NewExpressionList(
    Token* keyword,
    const std::vector<Expression*>& expressions) {
  auto const node = new (zone()) ExpressionList(keyword, expressions);
  for (auto const expression : expressions)
    SetParent(expression, node);
  return node;
}

ExpressionStatement* Factory::NewExpressionStatement(Expression* expression) {
  auto const node = new (zone()) ExpressionStatement(expression);
  SetParent(expression, node);
  return node;
}

ForEachStatement* Factory::NewForEachStatement(Token* keyword,
                                               Variable* variable,
                                               Expression* enumerable,
                                               Statement* statement) {
  auto const node =
      new (zone()) ForEachStatement(keyword, variable, enumerable, statement);
  SetParent(enumerable, node);
  SetParent(statement, node);
  return node;
}

ForStatement* Factory::NewForStatement(Token* keyword,
                                       Statement* initializer,
                                       Expression* condition,
                                       Statement* step,
                                       Statement* statement) {
  auto const node = new (zone())
      ForStatement(keyword, initializer, condition, step, statement);
  if (initializer)
    SetParent(initializer, node);
  if (condition)
    SetParent(condition, node);
  if (statement)
    SetParent(statement, node);
  return node;
}

IfStatement* Factory::NewIfStatement(Token* keyword,
                                     Expression* condition,
                                     Statement* then_statement,
                                     Statement* else_statement) {
  auto const node = new (zone())
      IfStatement(keyword, condition, then_statement, else_statement);
  SetParent(condition, node);
  SetParent(then_statement, node);
  if (else_statement)
    SetParent(else_statement, node);
  return node;
}

InvalidStatement* Factory::NewInvalidStatement(Token* token) {
  return new (zone()) InvalidStatement(token);
}

ReturnStatement* Factory::NewReturnStatement(Token* keyword,
                                             Expression* value) {
  auto const node = new (zone()) ReturnStatement(keyword, value);
  if (value)
    SetParent(value, node);
  return node;
}

ThrowStatement* Factory::NewThrowStatement(Token* keyword, Expression* value) {
  auto const node = new (zone()) ThrowStatement(keyword, value);
  if (value)
    SetParent(value, node);
  return node;
}

TryStatement* Factory::NewTryStatement(
    Token* keyword,
    BlockStatement* protected_block,
    const std::vector<CatchClause*>& catch_clauses,
    BlockStatement* finally_block) {
  auto const node = new (zone()) TryStatement(zone(), keyword, protected_block,
                                              catch_clauses, finally_block);
  SetParent(protected_block, node);
  if (finally_block)
    SetParent(finally_block, node);
  return node;
}

UsingStatement* Factory::NewUsingStatement(Token* keyword,
                                           Variable* variable,
                                           Expression* resource,
                                           Statement* statement) {
  auto const node =
      new (zone()) UsingStatement(keyword, variable, resource, statement);
  SetParent(resource, node);
  SetParent(statement, node);
  return node;
}

VarDeclaration* Factory::NewVarDeclaration(Token* token,
                                           Variable* variable,
                                           Expression* expression) {
  auto const node = new (zone()) VarDeclaration(token, variable, expression);
  SetParent(expression, node);
  return node;
}

VarStatement* Factory::NewVarStatement(
    Token* keyword,
    const std::vector<VarDeclaration*>& declarations) {
  auto const node = new (zone()) VarStatement(zone(), keyword, declarations);
  for (auto const declaration : declarations) {
    if (!declaration->variable()->type()->parent())
      SetParent(declaration->variable()->type(), node);
    SetParent(declaration, node);
  }
  return node;
}

WhileStatement* Factory::NewWhileStatement(Token* keyword,
                                           Expression* condition,
                                           Statement* statement) {
  auto const node = new (zone()) WhileStatement(keyword, condition, statement);
  SetParent(condition, node);
  SetParent(statement, node);
  return node;
}

YieldStatement* Factory::NewYieldStatement(Token* keyword, Expression* value) {
  auto const node = new (zone()) YieldStatement(keyword, value);
  SetParent(value, node);
  return node;
}

//////////////////////////////////////////////////////////////////////
//
// Types
//
ArrayType* Factory::NewArrayType(Token* op,
                                 Type* element_type,
                                 const std::vector<int>& dimensions) {
  auto const node =
      new (zone()) ArrayType(zone(), op, element_type, dimensions);
  SetParent(element_type, node);
  return node;
}

ConstructedType* Factory::NewConstructedType(ConstructedName* reference) {
  auto const node = new (zone()) ConstructedType(reference);
  SetParent(reference, node);
  return node;
}

InvalidType* Factory::NewInvalidType(Expression* expression) {
  auto const node = new (zone()) InvalidType(expression);
  SetParent(expression, node);
  return node;
}

OptionalType* Factory::NewOptionalType(Token* token, Type* base_type) {
  auto const node = new (zone()) OptionalType(token, base_type);
  SetParent(base_type, node);
  return node;
}

TypeMemberAccess* Factory::NewTypeMemberAccess(NamespaceBody* namespace_body,
                                               MemberAccess* reference) {
  auto const node = NewTypeMemberAccess(reference);
  SetParent(node, namespace_body);
  return node;
}

TypeMemberAccess* Factory::NewTypeMemberAccess(MemberAccess* reference) {
  auto const node = new (zone()) TypeMemberAccess(reference);
  SetParent(reference, node);
  return node;
}

TypeNameReference* Factory::NewTypeNameReference(NamespaceBody* namespace_body,
                                                 NameReference* reference) {
  auto const node = NewTypeNameReference(reference);
  SetParent(node, namespace_body);
  return node;
}

TypeNameReference* Factory::NewTypeNameReference(NameReference* reference) {
  auto const node = new (zone()) TypeNameReference(reference);
  SetParent(reference, node);
  return node;
}

TypeVariable* Factory::NewTypeVariable(Token* token) {
  return new (zone()) TypeVariable(token);
}

//////////////////////////////////////////////////////////////////////
//
// Misc
//
Parameter* Factory::NewParameter(Method* owner,
                                 ParameterKind kind,
                                 int position,
                                 Type* type,
                                 Token* name,
                                 Expression* value) {
  auto const node =
      new (zone()) Parameter(owner, kind, position, type, name, value);
  SetParent(type, owner);
  if (value)
    SetParent(value, owner);
  return node;
}

Variable* Factory::NewVariable(Token* keyword, Type* type, Token* name) {
  return new (zone()) Variable(keyword, type, name);
}

//////////////////////////////////////////////////////////////////////
//
// Utility
//
Node* Factory::RememberNode(Node* node) {
  return node;
}

void Factory::SetParent(Node* child, Node* parent) {
  DCHECK(!child->parent_) << *child << " old:" << *child->parent_;
  DCHECK_NE(child, parent);
  child->parent_ = parent;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
