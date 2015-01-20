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

// Represents constructed name
//  Name '<' Type (',' Type)* '>'
class ConstructedName final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(ConstructedName, Expression);

 public:
  const ZoneVector<Type*>& arguments() const { return arguments_; }
  NameReference* reference() const { return reference_; }

 private:
  ConstructedName(Zone* zone,
                  NameReference* reference,
                  const std::vector<Type*>& arguments);

  const ZoneVector<Type*> arguments_;
  NameReference* const reference_;

  DISALLOW_COPY_AND_ASSIGN(ConstructedName);
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

// Represents parameter reference
class ParameterReference final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(ParameterReference, Expression);

 public:
  // Returns name token where parameter referenced.
  Token* name() const { return token(); }
  Parameter* parameter() const { return parameter_; }

 private:
  ParameterReference(Token* name, Parameter* parameter);

  Parameter* const parameter_;

  DISALLOW_COPY_AND_ASSIGN(ParameterReference);
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

// Represents locally declared variable.
class Variable final : public NamedNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(Variable, NamedNode);

 public:
  bool is_const() const;
  Type* type() const { return type_; }
  Expression* value() const { return value_; }

 private:
  // |keyword| one of 'catch', 'const', 'for', 'using', or |nullptr|
  Variable(Token* keyword, Type* type, Token* name, Expression* value);

  Type* const type_;
  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

// Represents local variable reference
class VariableReference final : public Expression {
  DECLARE_CONCRETE_AST_NODE_CLASS(VariableReference, Expression);

 public:
  // Returns name token where local variable referenced.
  Token* name() const { return token(); }
  Variable* variable() const { return variable_; }

 private:
  VariableReference(Token* name, Variable* variable);

  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(VariableReference);
};

//////////////////////////////////////////////////////////////////////
//
// Types
//
class Type : public Expression {
  DECLARE_ABSTRACT_AST_NODE_CLASS(Type, Expression);

 protected:
  explicit Type(Token* op);

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

// Represents ArrayType expression:
//  PrimaryExpresion Rank+
//  Rank ::= '[' ','* ']'
class ArrayType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(ArrayType, Type);

 public:
  Expression* element_type() const { return element_type_; }
  const ZoneVector<int>& ranks() const { return ranks_; }

 private:
  ArrayType(Zone* zone,
            Token* op_token,
            Type* element_type,
            const std::vector<int>& ranks);

  Type* const element_type_;
  const ZoneVector<int> ranks_;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

// ConstructedType
class ConstructedType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(ConstructedType, Type);

 public:
  ConstructedName* reference() const { return reference_; }

 private:
  explicit ConstructedType(ConstructedName* reference);

  ConstructedName* const reference_;

  DISALLOW_COPY_AND_ASSIGN(ConstructedType);
};

// InvalidType
class InvalidType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(InvalidType, Type);

 public:
  Expression* expression() const { return expression_; }

 private:
  explicit InvalidType(Expression* expression);

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(InvalidType);
};

// OptionalType
class OptionalType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(OptionalType, Type);

 public:
  Type* base_type() const { return base_type_; }

 private:
  OptionalType(Token* op, Type* base_type);

  Type* const base_type_;

  DISALLOW_COPY_AND_ASSIGN(OptionalType);
};

// TypeMemberAccess
class TypeMemberAccess final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(TypeMemberAccess, Type);

 public:
  MemberAccess* reference() const { return reference_; }

 private:
  explicit TypeMemberAccess(MemberAccess* reference);

  MemberAccess* const reference_;

  DISALLOW_COPY_AND_ASSIGN(TypeMemberAccess);
};

// TypeNameReference
class TypeNameReference final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(TypeNameReference, Type);

 public:
  Token* name() const;
  NameReference* reference() const { return reference_; }

 private:
  explicit TypeNameReference(NameReference* reference);

  NameReference* const reference_;

  DISALLOW_COPY_AND_ASSIGN(TypeNameReference);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_EXPRESSIONS_H_
