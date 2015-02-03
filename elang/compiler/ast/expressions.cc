// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/expressions.h"

#include "base/logging.h"

#include "elang/compiler/ast/types.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Expression
//
Expression::Expression(Token* op) : Node(nullptr, op) {
}

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
    : Expression(op), left_(left), right_(right) {
}

// Binary operation
BinaryOperation::BinaryOperation(Token* op, Expression* left, Expression* right)
    : Expression(op), left_(left), right_(right) {
  CHECK(is_arithmetic() || is_bitwise() || is_bitwise_shift() ||
        is_conditional() || is_equality() || is_relational());
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
    : Expression(op),
      condition_(condition),
      false_expression_(false_expression),
      true_expression_(true_expression) {
}

// ConstructedName
ConstructedName::ConstructedName(Zone* zone,
                                 NameReference* reference,
                                 const std::vector<Type*>& args)
    : Expression(reference->name()),
      arguments_(zone, args),
      reference_(reference) {
  DCHECK(!arguments_.empty());
}

// InvalidExpression
InvalidExpression::InvalidExpression(Token* token) : Expression(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

Literal::Literal(Token* literal) : Expression(literal) {
}

MemberAccess::MemberAccess(Zone* zone,
                           Token* name,
                           const std::vector<Expression*>& components)
    : Expression(name), components_(zone, components) {
  DCHECK_GE(components.size(), 2u);
}

NameReference::NameReference(Token* name) : Expression(name) {
  DCHECK(name->is_name() || name->is_type_name() || name == TokenType::Var);
}

// ParameterReference
ParameterReference::ParameterReference(Token* name, Parameter* parameter)
    : Expression(name), parameter_(parameter) {
}

UnaryOperation::UnaryOperation(Token* op, Expression* expression)
    : Expression(op), expression_(expression) {
}

// Variable
Variable::Variable(Token* keyword, Type* type, Token* name, Expression* value)
    : NamedNode(nullptr, keyword, name), type_(type), value_(value) {
  DCHECK(keyword == type->token() || keyword == TokenType::Const ||
         keyword == TokenType::Catch || keyword == TokenType::For ||
         keyword == TokenType::Using)
      << *keyword << " " << *type << *name;
}

bool Variable::is_const() const {
  return token() == TokenType::Const || token() == TokenType::Using;
}

// VariableReference
VariableReference::VariableReference(Token* name, Variable* variable)
    : Expression(name), variable_(variable) {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
