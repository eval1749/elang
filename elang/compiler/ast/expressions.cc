// Copyright 2014 Project Vogue. All rights reserved.
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
Expression::Expression(Token* op) : Node(op) {
}

ArrayType::ArrayType(Zone* zone,
                     Token* op,
                     Expression* element_type,
                     const std::vector<int>& ranks)
    : Expression(op), element_type_(element_type), ranks_(zone, ranks) {
}

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
                         Expression* cond_expr,
                         Expression* then_expr,
                         Expression* else_expr)
    : Expression(op), cond_(cond_expr), else_(else_expr), then_(then_expr) {
}

ConstructedType::ConstructedType(Zone* zone,
                                 Expression* type,
                                 const std::vector<Expression*>& args)
    : Expression(type->token()), arguments_(zone, args), blueprint_type_(type) {
  DCHECK(!arguments_.empty());
}

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

UnaryOperation::UnaryOperation(Token* op, Expression* expression)
    : Expression(op), expression_(expression) {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang