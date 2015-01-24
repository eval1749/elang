// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/expressions.h"

#include "base/logging.h"

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

BinaryOperation::BinaryOperation(Token* op, Expression* left, Expression* right)
    : Expression(op), left_(left), right_(right) {
}

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
  DCHECK(!keyword || keyword == TokenType::Const ||
         keyword == TokenType::Catch || keyword == TokenType::For ||
         keyword == TokenType::Using);
}

bool Variable::is_const() const {
  return token() == TokenType::Const || token() == TokenType::Using;
}

// VariableReference
VariableReference::VariableReference(Token* name, Variable* variable)
    : Expression(name), variable_(variable) {
}

//////////////////////////////////////////////////////////////////////
//
// Types
//

// Type
Type::Type(Token* token) : Expression(token) {
}

// ArrayType
ArrayType::ArrayType(Zone* zone,
                     Token* op,
                     Type* element_type,
                     const std::vector<int>& ranks)
    : Type(op), element_type_(element_type), ranks_(zone, ranks) {
}

// ConstructedType
ConstructedType::ConstructedType(ConstructedName* reference)
    : Type(reference->token()), reference_(reference) {
}

// InvalidType
InvalidType::InvalidType(Expression* expression)
    : Type(expression->token()), expression_(expression) {
}

// OptionalType
OptionalType::OptionalType(Token* op, Type* base_type)
    : Type(op), base_type_(base_type) {
  DCHECK_EQ(op, TokenType::OptionalType);
}

// TypeMemberAccess
TypeMemberAccess::TypeMemberAccess(MemberAccess* reference)
    : Type(reference->token()), reference_(reference) {
}

// TypeNameReference
TypeNameReference::TypeNameReference(NameReference* reference)
    : Type(reference->token()), reference_(reference) {
}

Token* TypeNameReference::name() const {
  return reference_->name();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
