// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_EXPRESSIONS_H_
#define ELANG_COMPILER_AST_EXPRESSIONS_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Expression
//
class Expression : public Node {
  DECLARE_ABSTRACT_AST_NODE_CLASS(Expression, Node);

 public:
  Token* op() const { return token(); }

 protected:
  explicit Expression(Token* op);

 private:
  DISALLOW_COPY_AND_ASSIGN(Expression);
};

// Represents array access, e.g. array[index (',' index)*]
class ArrayAccess final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(ArrayAccess, Expression);

 public:
  Expression* array() const { return array_; }
  const ZoneVector<Expression*>& indexes() const { return indexes_; }

 private:
  ArrayAccess(Zone* zone,
              Token* name,
              Expression* array,
              const std::vector<Expression*>& indexes);

  Expression* array_;
  const ZoneVector<Expression*> indexes_;

  DISALLOW_COPY_AND_ASSIGN(ArrayAccess);
};

// Represents ArrayType expression:
//  PrimaryExpresion Rank+
//  Rank ::= '[' ','* ']'
class ArrayType final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(ArrayType, Expression);

 public:
  Expression* element_type() const { return element_type_; }
  const ZoneVector<int>& ranks() const { return ranks_; }

 private:
  ArrayType(Zone* zone,
            Token* op_token,
            Expression* expression,
            const std::vector<int>& ranks);

  // Node
  bool is_type() const final;

  Expression* const element_type_;
  const ZoneVector<int> ranks_;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

// Represents assignment:
//  UnaryExpresion AssignmentOperator Expression
//  AssignmentOperator ::= '=' | '+=' | ...
class Assignment final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(Assignment, Expression);

 public:
  Expression* left() const { return left_; }
  Expression* right() const { return right_; }

 private:
  Assignment(Token* op, Expression* left, Expression* right);

  Expression* const left_;
  Expression* const right_;

  DISALLOW_COPY_AND_ASSIGN(Assignment);
};

// Represents BinaryOperation
class BinaryOperation final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(BinaryOperation, Expression);

 public:
  Expression* left() const { return left_; }
  Expression* right() const { return right_; }

 private:
  BinaryOperation(Token* op, Expression* left, Expression* right);

  Expression* const left_;
  Expression* const right_;

  DISALLOW_COPY_AND_ASSIGN(BinaryOperation);
};

// Represents call expression:
//  PrimaryExpresion '(' ArgumentList? ')'
//  ArgumentList ::= Expression | Expression (',' Expression) *
class Call final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(Call, Expression);

 public:
  const ZoneVector<Expression*>& arguments() const { return arguments_; }
  Expression* callee() const { return callee_; }

 private:
  Call(Zone* zone,
       Expression* callee,
       const std::vector<Expression*>& arguments);

  Expression* const callee_;
  const ZoneVector<Expression*> arguments_;

  DISALLOW_COPY_AND_ASSIGN(Call);
};

// Represents conditional expression:
//  Expression '?' Expression ":' Expression
class Conditional final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(Conditional, Expression);

 public:
  Expression* conditional() const { return cond_; }
  Expression* else_expression() const { return else_; }
  Expression* then_expression() const { return then_; }

 private:
  Conditional(Token* op,
              Expression* condition,
              Expression* then_expression,
              Expression* else_expression);

  Expression* const cond_;
  Expression* const else_;
  Expression* const then_;

  DISALLOW_COPY_AND_ASSIGN(Conditional);
};

// Represents constructed type:
//  Type '<' Type (',' Type)* '>'
class ConstructedType final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(ConstructedType, Expression);

 public:
  const ZoneVector<Expression*>& arguments() const { return arguments_; }
  Expression* blueprint_type() const { return blueprint_type_; }

 private:
  ConstructedType(Zone* zone,
                  Expression* expression,
                  const std::vector<Expression*>& arguments);

  // Node
  bool is_type() const final;

  const ZoneVector<Expression*> arguments_;
  Expression* const blueprint_type_;

  DISALLOW_COPY_AND_ASSIGN(ConstructedType);
};

// Represents invalid expression. This expression is used for continuing parsing
// after syntax error.
class InvalidExpression final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(InvalidExpression, Expression);

 private:
  explicit InvalidExpression(Token* token);

  DISALLOW_COPY_AND_ASSIGN(InvalidExpression);
};

class Literal final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(Literal, Expression);

 private:
  explicit Literal(Token* literal);

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

// Represents type components connected by '.', e.g. |G<S, T>.F<X>.A|.
class MemberAccess final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(MemberAccess, Expression);

 public:
  const ZoneVector<Expression*>& components() const { return components_; }

 private:
  MemberAccess(Zone* zone,
               Token* name,
               const std::vector<Expression*>& components);

  const ZoneVector<Expression*> components_;

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

// Represents unary expression:
//  '+' UnaryExpression |
//  '-' UnaryExpression |
//  '--' UnaryExpression |
//  '++' UnaryExpression |
//  'dynamic_cast' '<' Type '>' '(' Expression ')' |
//  'static_cast' '<' Type '>' '(' Expression ')'
class UnaryOperation final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(UnaryOperation, Expression);

 public:
  Expression* expression() const { return expression_; }

 private:
  UnaryOperation(Token* op, Expression* expression);

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(UnaryOperation);
};

// Represents local variable reference
class VariableReference final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(VariableReference, Expression);

 public:
  // Returns name token where local variable referenced.
  Token* name() const { return token(); }
  LocalVariable* variable() const { return variable_; }

 private:
  VariableReference(Token* name, LocalVariable* variable);

  LocalVariable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(VariableReference);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_EXPRESSIONS_H_
