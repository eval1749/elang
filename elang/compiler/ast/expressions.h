// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_EXPRESSIONS_H_
#define ELANG_COMPILER_AST_EXPRESSIONS_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/nodes.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Expression
//
class Expression : public Node {
  DECLARE_ABSTRACT_AST_NODE_CLASS(Expression, Node);

 public:
  Token* op() const { return token(); }

 protected:
  Expression(ContainerNode* container, Token* op);
  explicit Expression(Token* op);

 private:
  DISALLOW_COPY_AND_ASSIGN(Expression);
};

// Represents array access, e.g. array[index (',' index)*]
class ArrayAccess final : public VariadicNode<Expression> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ArrayAccess, Expression);

 public:
  Expression* array() const;
  ChildNodes<Expression> indexes() const;

 private:
  ArrayAccess(Zone* zone,
              Token* name,
              Expression* array,
              const std::vector<Expression*>& indexes);

  DISALLOW_COPY_AND_ASSIGN(ArrayAccess);
};

// Represents assignment:
//  UnaryExpresion AssignmentOperator Expression
//  AssignmentOperator ::= '=' | '+=' | ...
class Assignment final : public SimpleNode<Expression, 2> {
  DECLARE_CONCRETE_AST_NODE_CLASS(Assignment, Expression);

 public:
  Expression* left() const;
  Expression* right() const;

 private:
  Assignment(Token* op, Expression* left, Expression* right);

  DISALLOW_COPY_AND_ASSIGN(Assignment);
};

// Represents BinaryOperation
class BinaryOperation final : public SimpleNode<Expression, 2> {
  DECLARE_CONCRETE_AST_NODE_CLASS(BinaryOperation, Expression);

 public:
  bool is_arithmetic() const;
  bool is_bitwise() const;
  bool is_bitwise_shift() const;
  bool is_conditional() const;
  bool is_equality() const;
  bool is_relational() const;
  Expression* left() const;
  Expression* right() const;

 private:
  BinaryOperation(Token* op, Expression* left, Expression* right);

  DISALLOW_COPY_AND_ASSIGN(BinaryOperation);
};

// Represents call expression:
//  PrimaryExpresion '(' ArgumentList? ')'
//  ArgumentList ::= Expression | Expression (',' Expression) *
class Call final : public VariadicNode<Expression> {
  DECLARE_CONCRETE_AST_NODE_CLASS(Call, Expression);

 public:
  int arity() const;
  ChildNodes<Expression> arguments() const;
  Expression* callee() const;

 private:
  Call(Zone* zone,
       Expression* callee,
       const std::vector<Expression*>& arguments);

  DISALLOW_COPY_AND_ASSIGN(Call);
};

// Represents conditional expression:
//  Expression '?' Expression ":' Expression
class Conditional final : public SimpleNode<Expression, 3> {
  DECLARE_CONCRETE_AST_NODE_CLASS(Conditional, Expression);

 public:
  Expression* condition() const;
  Expression* false_expression() const;
  Expression* true_expression() const;

 private:
  Conditional(Token* op,
              Expression* condition,
              Expression* true_expression,
              Expression* false_expression);

  DISALLOW_COPY_AND_ASSIGN(Conditional);
};

// Represents constructed name
//  Name '<' Type (',' Type)* '>'
class ConstructedName final : public VariadicNode<Expression> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ConstructedName, Expression);

 public:
  ChildNodes<Type> arguments() const;
  Expression* reference() const;

 private:
  ConstructedName(Zone* zone,
                  Expression* reference,
                  const std::vector<Type*>& arguments);

  DISALLOW_COPY_AND_ASSIGN(ConstructedName);
};

// Represents unary expression:
//  '--' UnaryExpression |
//  '++' UnaryExpression |
//  UnaryExpression '--' |
//  UnaryExpression '++'
class IncrementExpression final : public SimpleNode<Expression, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(IncrementExpression, Expression);

 public:
  Expression* expression() const;

 private:
  IncrementExpression(Token* op, Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(IncrementExpression);
};

// Represents invalid expression. This expression is used for continuing parsing
// after syntax error.
class InvalidExpression final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(InvalidExpression, Expression);

 private:
  explicit InvalidExpression(Token* token);

  DISALLOW_COPY_AND_ASSIGN(InvalidExpression);
};

// Represent literal value. |token()| returns literal data as |Token|.
class Literal final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(Literal, Expression);

 private:
  explicit Literal(Token* literal);

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

// Represents member acesss, e.g. |container.member|.
class MemberAccess final : public SimpleNode<Expression, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(MemberAccess, Expression);

 public:
  Expression* container() const;
  Token* member() const { return member_; }

 private:
  MemberAccess(Expression* container, Token* member);

  Token* const member_;

  DISALLOW_COPY_AND_ASSIGN(MemberAccess);
};

// Represents non-local name reference
class NameReference final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(NameReference, Expression);

 public:
  Token* name() const { return token(); }

 private:
  explicit NameReference(Token* name);

  DISALLOW_COPY_AND_ASSIGN(NameReference);
};

// Represents no-expression for field class member.
class NoExpression final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(NoExpression, Expression);

 private:
  explicit NoExpression(Token* token);

  DISALLOW_COPY_AND_ASSIGN(NoExpression);
};

// Represents parameter reference
class ParameterReference final : public SimpleNode<Expression, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ParameterReference, Expression);

 public:
  // Returns name token where parameter referenced.
  Token* name() const { return token(); }
  Parameter* parameter() const;

 private:
  ParameterReference(Token* name, Parameter* parameter);

  DISALLOW_COPY_AND_ASSIGN(ParameterReference);
};

// Represents unary expression:
//  '+' UnaryExpression |
//  '-' UnaryExpression |
//  '--' UnaryExpression |
//  '++' UnaryExpression |
//  'dynamic_cast' '<' Type '>' '(' Expression ')' |
//  'static_cast' '<' Type '>' '(' Expression ')'
class UnaryOperation final : public SimpleNode<Expression, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(UnaryOperation, Expression);

 public:
  Expression* expression() const;

 private:
  UnaryOperation(Token* op, Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(UnaryOperation);
};

// Represents locally declared variable.
class Variable final : public NamedNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(Variable, NamedNode);

 public:
  bool is_const() const;
  Type* type() const { return type_; }

 private:
  // |keyword| one of 'catch', 'const', 'for', 'using', or |type|.
  Variable(Token* keyword, Type* type, Token* name);

  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

// Represents local variable reference
class VariableReference final : public SimpleNode<Expression, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(VariableReference, Expression);

 public:
  // Returns name token where local variable referenced.
  Token* name() const { return token(); }
  Variable* variable() const;

 private:
  VariableReference(Token* name, Variable* variable);

  DISALLOW_COPY_AND_ASSIGN(VariableReference);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_EXPRESSIONS_H_
