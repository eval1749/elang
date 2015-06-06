// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/expressions.h"

#include "base/logging.h"

#include "elang/compiler/ast/container_node.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

// ArrayAccess
ArrayAccess::ArrayAccess(Zone* zone,
                         Token* bracket,
                         Expression* array,
                         const std::vector<Expression*>& indexes)
    : Expression(bracket), array_(array), indexes_(zone, indexes) {
  DCHECK_EQ(bracket, TokenType::LeftSquareBracket);
  DCHECK(!indexes.empty());
}

// Assignment
Assignment::Assignment(Token* op, Expression* left, Expression* right)
    : SimpleNode(op) {
  set_child_at(0, left);
  set_child_at(1, right);
}

ast::Expression* Assignment::left() const {
  return child_at(0)->as<ast::Expression>();
}

ast::Expression* Assignment::right() const {
  return child_at(1)->as<ast::Expression>();
}

// Binary operation
BinaryOperation::BinaryOperation(Token* op, Expression* left, Expression* right)
    : SimpleNode(op) {
  CHECK(is_arithmetic() || is_bitwise() || is_bitwise_shift() ||
        is_conditional() || is_equality() || is_relational());
  set_child_at(0, left);
  set_child_at(1, right);
}

bool BinaryOperation::is_arithmetic() const {
  return op() == TokenType::Add || op() == TokenType::Div ||
         op() == TokenType::Mod || op() == TokenType::Mul ||
         op() == TokenType::Sub;
}

bool BinaryOperation::is_bitwise() const {
  return op() == TokenType::BitAnd || op() == TokenType::BitOr ||
         op() == TokenType::BitXor;
}

bool BinaryOperation::is_bitwise_shift() const {
  return op() == TokenType::Shl || op() == TokenType::Shr;
}

bool BinaryOperation::is_conditional() const {
  return op() == TokenType::And || op() == TokenType::NullOr ||
         op() == TokenType::Or;
}

bool BinaryOperation::is_equality() const {
  return op() == TokenType::Eq || op() == TokenType::Ne;
}

bool BinaryOperation::is_relational() const {
  return op() == TokenType::Ge || op() == TokenType::Gt ||
         op() == TokenType::Le || op() == TokenType::Lt;
}

ast::Expression* BinaryOperation::left() const {
  return child_at(0)->as<ast::Expression>();
}

ast::Expression* BinaryOperation::right() const {
  return child_at(1)->as<ast::Expression>();
}

// Call
Call::Call(Zone* zone,
           Expression* callee,
           const std::vector<Expression*>& arguments)
    : Expression(callee->token()),
      arguments_(zone, arguments),
      callee_(callee) {
}

Conditional::Conditional(Token* op,
                         Expression* condition,
                         Expression* true_expression,
                         Expression* false_expression)
    : SimpleNode(op) {
  set_child_at(0, condition);
  set_child_at(1, false_expression);
  set_child_at(2, true_expression);
}

Expression* Conditional::condition() const {
  return child_at(0)->as<ast::Expression>();
}

Expression* Conditional::false_expression() const {
  return child_at(1)->as<ast::Expression>();
}

Expression* Conditional::true_expression() const {
  return child_at(2)->as<ast::Expression>();
}

// ConstructedName
ConstructedName::ConstructedName(Zone* zone,
                                 Expression* reference,
                                 const std::vector<Type*>& args)
    : Expression(reference->name()),
      arguments_(zone, args),
      reference_(reference) {
  DCHECK(!arguments_.empty());
}

// Expression
Expression::Expression(ContainerNode* container, Token* op)
    : Node(container, op) {
}

Expression::Expression(Token* op) : Expression(nullptr, op) {
}

// IncrementExpression
IncrementExpression::IncrementExpression(Token* op, Expression* expression)
    : SimpleNode(op) {
  set_child_at(0, expression);
}

Expression* IncrementExpression::expression() const {
  return child_at(0)->as<ast::Expression>();
}

// InvalidExpression
InvalidExpression::InvalidExpression(Token* token) : Expression(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

Literal::Literal(Token* literal) : Expression(literal) {
}

// MemberAccess
MemberAccess::MemberAccess(Expression* container, Token* member)
    : SimpleNode(member), member_(member) {
  set_child_at(0, container);
}

Expression* MemberAccess::container() const {
  return child_at(0)->as<ast::Expression>();
}

// NameReference
NameReference::NameReference(Token* name) : Expression(name) {
  DCHECK(name->is_name() || name->is_type_name() || name == TokenType::Var)
      << name;
}

// NoExpression
NoExpression::NoExpression(Token* token) : Expression(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

// ParameterReference
ParameterReference::ParameterReference(Token* name, Parameter* parameter)
    : SimpleNode(name) {
  set_child_at(0, parameter);
}

Parameter* ParameterReference::parameter() const {
  return child_at(0)->as<ast::Parameter>();
}

// UnaryOperation
UnaryOperation::UnaryOperation(Token* op, Expression* expression)
    : SimpleNode(op) {
  set_child_at(0, expression);
}

Expression* UnaryOperation::expression() const {
  return child_at(0)->as<ast::Expression>();
}

// Variable
Variable::Variable(Token* keyword, Type* type, Token* name)
    : NamedNode(nullptr, keyword, name), type_(type) {
  DCHECK(keyword == TokenType::Const || keyword == TokenType::Catch ||
         keyword == TokenType::For || keyword == TokenType::Using ||
         keyword == TokenType::Var)
      << *keyword << " " << *type << *name;
}

bool Variable::is_const() const {
  return token() == TokenType::Const || token() == TokenType::Using;
}

// VariableReference
VariableReference::VariableReference(Token* name, Variable* variable)
    : SimpleNode(name) {
  set_child_at(0, variable);
}

Variable* VariableReference::variable() const {
  return child_at(0)->as<ast::Variable>();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
