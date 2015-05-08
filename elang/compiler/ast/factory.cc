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
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

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
Factory::Factory(Zone* zone) : zone_(zone) {
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
  return new (zone_) Alias(namespace_body, keyword, alias_name, reference);
}

Class* Factory::NewClass(NamespaceNode* outer,
                         Modifiers modifiers,
                         Token* keyword,
                         Token* name) {
  return new (zone_) Class(zone_, outer, modifiers, keyword, name);
}

ClassBody* Factory::NewClassBody(BodyNode* outer,
                                 Class* owner,
                                 const std::vector<Type*>& base_class_names) {
  auto const node =
      new (zone_) ClassBody(zone_, outer, owner, base_class_names);
  for (auto const base_class_name : base_class_names)
    SetParent(base_class_name, node);
  return node;
}

Enum* Factory::NewEnum(BodyNode* container,
                       Modifiers modifiers,
                       Token* keyword,
                       Token* name) {
  return new (zone_) Enum(zone_, container, modifiers, keyword, name);
}

EnumMember* Factory::NewEnumMember(Enum* owner,
                                   Token* name,
                                   int position,
                                   Expression* expression) {
  return new (zone_) EnumMember(owner, name, position, expression);
}

Field* Factory::NewField(ClassBody* outer,
                         Modifiers modifiers,
                         Type* type,
                         Token* name,
                         Expression* expression) {
  return new (zone_) Field(outer, modifiers, type, name, expression);
}

Import* Factory::NewImport(NamespaceBody* namespace_body,
                           Token* keyword,
                           Expression* reference) {
  return new (zone_) Import(namespace_body, keyword, reference);
}

Method* Factory::NewMethod(ClassBody* outer,
                           MethodGroup* method_group,
                           Modifiers modifies,
                           Type* type,
                           Token* name,
                           const std::vector<Token*>& type_parameters) {
  return new (zone_)
      Method(zone_, outer, method_group, modifies, type, name, type_parameters);
}

MethodBody* Factory::NewMethodBody(Method* method) {
  return new (zone_) MethodBody(zone_, method);
}

MethodGroup* Factory::NewMethodGroup(Class* owner, Token* name) {
  DCHECK(name->is_name());
  return new (zone_) MethodGroup(zone_, owner, name);
}

Namespace* Factory::NewNamespace(Namespace* outer,
                                 Token* keyword,
                                 Token* name) {
  DCHECK_EQ(keyword->type(), TokenType::Namespace);
  return new (zone_) Namespace(zone_, outer, keyword, name);
}

NamespaceBody* Factory::NewNamespaceBody(NamespaceBody* outer,
                                         Namespace* owner) {
  return new (zone_) NamespaceBody(zone_, outer, owner);
}

//////////////////////////////////////////////////////////////////////
//
// Expression nodes
//
ArrayAccess* Factory::NewArrayAccess(Token* bracket,
                                     Expression* array,
                                     const std::vector<Expression*> indexes) {
  auto const node = new (zone_) ArrayAccess(zone_, bracket, array, indexes);
  SetParent(array, node);
  for (auto const index : indexes)
    SetParent(index, node);
  return node;
}

Assignment* Factory::NewAssignment(Token* op,
                                   Expression* left,
                                   Expression* right) {
  auto const node = new (zone_) Assignment(op, left, right);
  SetParent(left, node);
  SetParent(right, node);
  return node;
}

BinaryOperation* Factory::NewBinaryOperation(Token* op,
                                             Expression* left,
                                             Expression* right) {
  auto const node = new (zone_) BinaryOperation(op, left, right);
  SetParent(left, node);
  SetParent(right, node);
  return node;
}

Conditional* Factory::NewConditional(Token* op,
                                     Expression* cond_expr,
                                     Expression* then_expr,
                                     Expression* else_expr) {
  auto const node =
      new (zone_) Conditional(op, cond_expr, then_expr, else_expr);
  SetParent(cond_expr, node);
  SetParent(then_expr, node);
  SetParent(else_expr, node);
  return node;
}

ConstructedName* Factory::NewConstructedName(
    NameReference* reference,
    const std::vector<Type*>& arguments) {
  auto const node = new (zone_) ConstructedName(zone_, reference, arguments);
  SetParent(reference, node);
  for (auto const argument : arguments)
    SetParent(argument, node);
  return node;
}

IncrementExpression* Factory::NewIncrementExpression(Token* op,
                                                     Expression* expr) {
  auto const node = new (zone_) IncrementExpression(op, expr);
  SetParent(expr, node);
  return node;
}

InvalidExpression* Factory::NewInvalidExpression(Token* token) {
  return new (zone_) InvalidExpression(token);
}

Literal* Factory::NewLiteral(Token* literal) {
  return new (zone_) Literal(literal);
}

MemberAccess* Factory::NewMemberAccess(
    Token* name,
    const std::vector<Expression*>& components) {
  auto const node = new (zone_) MemberAccess(zone_, name, components);
  for (auto const component : components) {
    component->parent_ = nullptr;
    SetParent(component, node);
  }
  return node;
}

NameReference* Factory::NewNameReference(Token* name) {
  return new (zone_) NameReference(name);
}

ParameterReference* Factory::NewParameterReference(Token* name,
                                                   Parameter* parameter) {
  return new (zone_) ParameterReference(name, parameter);
}

UnaryOperation* Factory::NewUnaryOperation(Token* op, Expression* expr) {
  auto const node = new (zone_) UnaryOperation(op, expr);
  SetParent(expr, node);
  return node;
}

VariableReference* Factory::NewVariableReference(Token* name,
                                                 Variable* variable) {
  return new (zone_) VariableReference(name, variable);
}

//////////////////////////////////////////////////////////////////////
//
// Statement
//
BlockStatement* Factory::NewBlockStatement(
    Token* keyword,
    const std::vector<Statement*> statements) {
  auto const node = new (zone_) BlockStatement(zone_, keyword, statements);
  for (auto const statement : statements)
    SetParent(statement, node);
  return node;
}

BreakStatement* Factory::NewBreakStatement(Token* keyword) {
  return new (zone_) BreakStatement(keyword);
}

Call* Factory::NewCall(Expression* callee,
                       const std::vector<Expression*> arguments) {
  auto const node = new (zone_) Call(zone_, callee, arguments);
  SetParent(callee, node);
  for (auto const argument : arguments)
    SetParent(argument, node);
  return node;
}

CatchClause* Factory::NewCatchClause(Token* keyword,
                                     Type* type,
                                     Variable* variable,
                                     BlockStatement* block) {
  auto const node = new (zone_) CatchClause(keyword, type, variable, block);
  SetParent(type, node);
  SetParent(block, node);
  return node;
}

ContinueStatement* Factory::NewContinueStatement(Token* keyword) {
  return new (zone_) ContinueStatement(keyword);
}

DoStatement* Factory::NewDoStatement(Token* keyword,
                                     Statement* statement,
                                     Expression* condition) {
  auto const node = new (zone_) DoStatement(keyword, statement, condition);
  SetParent(statement, node);
  SetParent(condition, node);
  return node;
}

EmptyStatement* Factory::NewEmptyStatement(Token* keyword) {
  return new (zone_) EmptyStatement(keyword);
}

ExpressionList* Factory::NewExpressionList(
    Token* keyword,
    const std::vector<Expression*>& expressions) {
  auto const node = new (zone_) ExpressionList(keyword, expressions);
  for (auto const expression : expressions)
    SetParent(expression, node);
  return node;
}

ExpressionStatement* Factory::NewExpressionStatement(Expression* expression) {
  auto const node = new (zone_) ExpressionStatement(expression);
  SetParent(expression, node);
  return node;
}

ForEachStatement* Factory::NewForEachStatement(Token* keyword,
                                               Variable* variable,
                                               Expression* enumerable,
                                               Statement* statement) {
  auto const node =
      new (zone_) ForEachStatement(keyword, variable, enumerable, statement);
  SetParent(enumerable, node);
  SetParent(statement, node);
  return node;
}

ForStatement* Factory::NewForStatement(Token* keyword,
                                       Statement* initializer,
                                       Expression* condition,
                                       Statement* step,
                                       Statement* statement) {
  auto const node = new (zone_)
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
  auto const node = new (zone_)
      IfStatement(keyword, condition, then_statement, else_statement);
  SetParent(condition, node);
  SetParent(then_statement, node);
  if (else_statement)
    SetParent(else_statement, node);
  return node;
}

InvalidStatement* Factory::NewInvalidStatement(Token* token) {
  return new (zone_) InvalidStatement(token);
}

ReturnStatement* Factory::NewReturnStatement(Token* keyword,
                                             Expression* value) {
  auto const node = new (zone_) ReturnStatement(keyword, value);
  if (value)
    SetParent(value, node);
  return node;
}

ThrowStatement* Factory::NewThrowStatement(Token* keyword, Expression* value) {
  auto const node = new (zone_) ThrowStatement(keyword, value);
  if (value)
    SetParent(value, node);
  return node;
}

TryStatement* Factory::NewTryStatement(
    Token* keyword,
    BlockStatement* protected_block,
    const std::vector<CatchClause*>& catch_clauses,
    BlockStatement* finally_block) {
  auto const node = new (zone_) TryStatement(zone_, keyword, protected_block,
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
      new (zone_) UsingStatement(keyword, variable, resource, statement);
  SetParent(resource, node);
  SetParent(statement, node);
  return node;
}

VarDeclaration* Factory::NewVarDeclaration(Token* token,
                                           Variable* variable,
                                           Expression* expression) {
  auto const node = new (zone_) VarDeclaration(token, variable, expression);
  SetParent(expression, node);
  return node;
}

VarStatement* Factory::NewVarStatement(
    Token* keyword,
    const std::vector<VarDeclaration*>& declarations) {
  auto const node = new (zone_) VarStatement(zone_, keyword, declarations);
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
  auto const node = new (zone_) WhileStatement(keyword, condition, statement);
  SetParent(condition, node);
  SetParent(statement, node);
  return node;
}

YieldStatement* Factory::NewYieldStatement(Token* keyword, Expression* value) {
  auto const node = new (zone_) YieldStatement(keyword, value);
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
  auto const node = new (zone_) ArrayType(zone_, op, element_type, dimensions);
  SetParent(element_type, node);
  return node;
}

ConstructedType* Factory::NewConstructedType(ConstructedName* reference) {
  auto const node = new (zone_) ConstructedType(reference);
  SetParent(reference, node);
  return node;
}

InvalidType* Factory::NewInvalidType(Expression* expression) {
  auto const node = new (zone_) InvalidType(expression);
  SetParent(expression, node);
  return node;
}

OptionalType* Factory::NewOptionalType(Token* token, Type* base_type) {
  auto const node = new (zone_) OptionalType(token, base_type);
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
  auto const node = new (zone_) TypeMemberAccess(reference);
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
  auto const node = new (zone_) TypeNameReference(reference);
  SetParent(reference, node);
  return node;
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
      new (zone_) Parameter(owner, kind, position, type, name, value);
  SetParent(type, owner);
  if (value)
    SetParent(value, owner);
  return node;
}

Variable* Factory::NewVariable(Token* keyword, Type* type, Token* name) {
  return new (zone_) Variable(keyword, type, name);
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
