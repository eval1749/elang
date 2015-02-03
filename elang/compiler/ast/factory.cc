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

ClassBody* Factory::NewClassBody(BodyNode* outer, Class* owner) {
  return new (zone_) ClassBody(zone_, outer, owner);
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
  return new (zone_) ArrayAccess(zone_, bracket, array, indexes);
}

Assignment* Factory::NewAssignment(Token* op,
                                   Expression* left,
                                   Expression* right) {
  return new (zone_) Assignment(op, left, right);
}

BinaryOperation* Factory::NewBinaryOperation(Token* op,
                                             Expression* left,
                                             Expression* right) {
  return new (zone_) BinaryOperation(op, left, right);
}

Conditional* Factory::NewConditional(Token* op,
                                     Expression* cond_expr,
                                     Expression* then_expr,
                                     Expression* else_expr) {
  return new (zone_) Conditional(op, cond_expr, then_expr, else_expr);
}

ConstructedName* Factory::NewConstructedName(
    NameReference* reference,
    const std::vector<Type*>& arguments) {
  return new (zone_) ConstructedName(zone_, reference, arguments);
}

InvalidExpression* Factory::NewInvalidExpression(Token* token) {
  return new (zone_) InvalidExpression(token);
}

Literal* Factory::NewLiteral(Token* literal) {
  return new (zone_) Literal(literal);
}

MemberAccess* Factory::NewMemberAccess(
    Token* name,
    const std::vector<Expression*>& members) {
  return new (zone_) MemberAccess(zone_, name, members);
}

NameReference* Factory::NewNameReference(Token* name) {
  return new (zone_) NameReference(name);
}

ParameterReference* Factory::NewParameterReference(Token* name,
                                                   Parameter* parameter) {
  return new (zone_) ParameterReference(name, parameter);
}

UnaryOperation* Factory::NewUnaryOperation(Token* op, Expression* expr) {
  return new (zone_) UnaryOperation(op, expr);
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
  return new (zone_) BlockStatement(zone_, keyword, statements);
}

BreakStatement* Factory::NewBreakStatement(Token* keyword) {
  return new (zone_) BreakStatement(keyword);
}

Call* Factory::NewCall(Expression* callee,
                       const std::vector<Expression*> arguments) {
  return new (zone_) Call(zone_, callee, arguments);
}

CatchClause* Factory::NewCatchClause(Token* keyword,
                                     Type* type,
                                     Variable* variable,
                                     BlockStatement* block) {
  return new (zone_) CatchClause(keyword, type, variable, block);
}

ContinueStatement* Factory::NewContinueStatement(Token* keyword) {
  return new (zone_) ContinueStatement(keyword);
}

DoStatement* Factory::NewDoStatement(Token* keyword,
                                     Statement* statement,
                                     Expression* condition) {
  return new (zone_) DoStatement(keyword, statement, condition);
}

EmptyStatement* Factory::NewEmptyStatement(Token* keyword) {
  return new (zone_) EmptyStatement(keyword);
}

ExpressionList* Factory::NewExpressionList(
    Token* keyword,
    const std::vector<Expression*>& expressions) {
  return new (zone_) ExpressionList(keyword, expressions);
}

ExpressionStatement* Factory::NewExpressionStatement(Expression* expression) {
  return new (zone_) ExpressionStatement(expression);
}

ForEachStatement* Factory::NewForEachStatement(Token* keyword,
                                               Variable* variable,
                                               Expression* enumerable,
                                               Statement* statement) {
  return new (zone_) ForEachStatement(keyword, variable, enumerable, statement);
}

ForStatement* Factory::NewForStatement(Token* keyword,
                                       Statement* initializer,
                                       Expression* condition,
                                       Statement* step,
                                       Statement* statement) {
  return new (zone_)
      ForStatement(keyword, initializer, condition, step, statement);
}

IfStatement* Factory::NewIfStatement(Token* keyword,
                                     Expression* condition,
                                     Statement* then_statement,
                                     Statement* else_statement) {
  return new (zone_)
      IfStatement(keyword, condition, then_statement, else_statement);
}

InvalidStatement* Factory::NewInvalidStatement(Token* token) {
  return new (zone_) InvalidStatement(token);
}

ReturnStatement* Factory::NewReturnStatement(Token* keyword,
                                             Expression* value) {
  return new (zone_) ReturnStatement(keyword, value);
}

ThrowStatement* Factory::NewThrowStatement(Token* keyword, Expression* value) {
  return new (zone_) ThrowStatement(keyword, value);
}

TryStatement* Factory::NewTryStatement(
    Token* keyword,
    BlockStatement* protected_block,
    const std::vector<CatchClause*>& catch_clauses,
    BlockStatement* finally_block) {
  return new (zone_) TryStatement(zone_, keyword, protected_block,
                                  catch_clauses, finally_block);
}

UsingStatement* Factory::NewUsingStatement(Token* keyword,
                                           Variable* variable,
                                           Expression* resource,
                                           Statement* statement) {
  return new (zone_) UsingStatement(keyword, variable, resource, statement);
}

VarStatement* Factory::NewVarStatement(
    Token* keyword,
    const std::vector<ast::Variable*>& variables) {
  return new (zone_) VarStatement(zone_, keyword, variables);
}

WhileStatement* Factory::NewWhileStatement(Token* keyword,
                                           Expression* condition,
                                           Statement* statement) {
  return new (zone_) WhileStatement(keyword, condition, statement);
}

YieldStatement* Factory::NewYieldStatement(Token* keyword, Expression* value) {
  return new (zone_) YieldStatement(keyword, value);
}

//////////////////////////////////////////////////////////////////////
//
// Types
//
ArrayType* Factory::NewArrayType(Token* op,
                                 Type* element_type,
                                 const std::vector<int>& dimensions) {
  return new (zone_) ArrayType(zone_, op, element_type, dimensions);
}

ConstructedType* Factory::NewConstructedType(ConstructedName* reference) {
  return new (zone_) ConstructedType(reference);
}

InvalidType* Factory::NewInvalidType(Expression* expression) {
  return new (zone_) InvalidType(expression);
}

OptionalType* Factory::NewOptionalType(Token* token, Type* base_type) {
  return new (zone_) OptionalType(token, base_type);
}

TypeMemberAccess* Factory::NewTypeMemberAccess(MemberAccess* reference) {
  return new (zone_) TypeMemberAccess(reference);
}

TypeNameReference* Factory::NewTypeNameReference(NameReference* reference) {
  return new (zone_) TypeNameReference(reference);
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
  return new (zone_) Parameter(owner, kind, position, type, name, value);
}

Variable* Factory::NewVariable(Token* keyword,
                               Type* type,
                               Token* name,
                               Expression* value) {
  return new (zone_) Variable(keyword, type, name, value);
}

//////////////////////////////////////////////////////////////////////
//
// Utility
//
Node* Factory::RememberNode(Node* node) {
  return node;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
