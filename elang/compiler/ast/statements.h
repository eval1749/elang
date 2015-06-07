// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_STATEMENTS_H_
#define ELANG_COMPILER_AST_STATEMENTS_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/nodes.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Statement
//
class Statement : public Node {
  DECLARE_ABSTRACT_AST_NODE_CLASS(Statement, Node);

 public:
  Token* keyword() const { return token(); }

 protected:
  explicit Statement(Token* keyword);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

// Represents block statement:
//  '{' Statement* '}'
class BlockStatement final : public VariadicNode<Statement> {
  DECLARE_CONCRETE_AST_NODE_CLASS(BlockStatement, Statement);

 public:
  ChildNodes<Statement> statements() const;

 private:
  // When last statement of this block statement is reachable, |keyword|
  // is left curry bracket, otherwise it is right curry bracket.
  BlockStatement(Zone* zone,
                 Token* keyword,
                 const std::vector<Statement*>& statements);

  DISALLOW_COPY_AND_ASSIGN(BlockStatement);
};

// Represents 'break' statement
//  'break' ';'
class BreakStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(BreakStatement, Statement);

 private:
  explicit BreakStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(BreakStatement);
};

// Represents 'catch' clause:
//  'catch' '(' Type Name? ')' '(' Statement* '}'
class CatchClause final : public SimpleNode<Node, 2> {
  DECLARE_CONCRETE_AST_NODE_CLASS(CatchClause, Node);

 public:
  BlockStatement* block() const;
  Type* type() const;
  Variable* variable() const { return variable_; }

 private:
  CatchClause(Token* keyword,
              Type* type,
              Variable* variable,
              BlockStatement* block);

  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(CatchClause);
};

// Represents 'continue' statement
//  'continue' ';'
class ContinueStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ContinueStatement, Statement);

 private:
  explicit ContinueStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(ContinueStatement);
};

// Represents 'do' statement:
//   'do' EmbededStatement 'while' '(' BooleanExpression ')' ';'
class DoStatement final : public SimpleNode<Statement, 2> {
  DECLARE_CONCRETE_AST_NODE_CLASS(DoStatement, Statement);

 public:
  Expression* condition() const;
  Statement* statement() const;

 private:
  DoStatement(Token* keyword, Statement* statement, Expression* condition);

  DISALLOW_COPY_AND_ASSIGN(DoStatement);
};

// Represents empty statement:
//  ';'
class EmptyStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(EmptyStatement, Statement);

 private:
  explicit EmptyStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(EmptyStatement);
};

// Represents comma separated expressions used in 'for' statement.
class ExpressionList : public VariadicNode<Statement> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ExpressionList, Statement);

 public:
  ChildNodes<Expression> expressions() const;

 protected:
  // Since |expressions| can be empty, we should have |keyword| paramter.
  ExpressionList(Zone* zone,
                 Token* keyword,
                 const std::vector<Expression*>& expressions);

 private:
  DISALLOW_COPY_AND_ASSIGN(ExpressionList);
};

// Represents expression statement:
//  Expression ';'
class ExpressionStatement final : public SimpleNode<Statement, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ExpressionStatement, Statement);

 public:
  Expression* expression() const;

 private:
  explicit ExpressionStatement(Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

// Represents 'for' + ':' statement.
class ForEachStatement final : public SimpleNode<Statement, 2> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ForEachStatement, Statement);

 public:
  Expression* enumerable() const;
  Statement* statement() const;
  Variable* variable() const { return variable_; }

 private:
  ForEachStatement(Token* keyword,
                   Variable* variable,
                   Expression* enumerable,
                   Statement* statement);

  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(ForEachStatement);
};

// Represents 'for' statement.
class ForStatement final : public SimpleNode<Statement, 4> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ForStatement, Statement);

 public:
  Expression* condition() const;
  Statement* initializer() const;
  Statement* statement() const;
  Statement* step() const;

 private:
  ForStatement(Token* keyword,
               Statement* initializer,
               Expression* condition,
               Statement* step,
               Statement* statement);

  DISALLOW_COPY_AND_ASSIGN(ForStatement);
};

// Represents 'if' statement:
//  'if' '(' BooleanExpression ')' EmbededStatement ('else' EmbededStatement)?
class IfStatement final : public SimpleNode<Statement, 3> {
  DECLARE_CONCRETE_AST_NODE_CLASS(IfStatement, Statement);

 public:
  Expression* condition() const;
  Statement* else_statement() const;
  Statement* then_statement() const;

 private:
  IfStatement(Token* keyword,
              Expression* condition,
              Statement* then_statement,
              Statement* else_statement);

  DISALLOW_COPY_AND_ASSIGN(IfStatement);
};

// Represents invalid statement. This statement is used for continuing parsing
// after syntax error.
class InvalidStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(InvalidStatement, Statement);

 private:
  explicit InvalidStatement(Token* token);

  DISALLOW_COPY_AND_ASSIGN(InvalidStatement);
};

// Represents no-statement for else claus, for initializer, and for steps.
class NoStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(NoStatement, Statement);

 private:
  explicit NoStatement(Token* token);

  DISALLOW_COPY_AND_ASSIGN(NoStatement);
};

// Represents 'return' statement:
//  'return' Expression? ';'
class ReturnStatement final : public SimpleNode<Statement, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ReturnStatement, Statement);

 public:
  Expression* expression() const;

 private:
  ReturnStatement(Token* keyword, Expression* value);

  DISALLOW_COPY_AND_ASSIGN(ReturnStatement);
};

// Represents 'throw' statement:
//  'throw' Expression
class ThrowStatement final : public SimpleNode<Statement, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(ThrowStatement, Statement);

 public:
  Expression* expression() const;

 private:
  ThrowStatement(Token* keyword, Expression* value);

  DISALLOW_COPY_AND_ASSIGN(ThrowStatement);
};

// Represents 'try' statement:
//  'try' BlockStatement (CatchCaluse*) ('finally' BlockStatement)?
class TryStatement final : public VariadicNode<Statement> {
  DECLARE_CONCRETE_AST_NODE_CLASS(TryStatement, Statement);

 public:
  ChildNodes<CatchClause> catch_clauses() const;
  BlockStatement* finally_block() const;
  BlockStatement* protected_block() const;

 private:
  TryStatement(Zone* zone,
               Token* keyword,
               BlockStatement* protected_block,
               const std::vector<CatchClause*>& catch_clauses,
               Statement* finally_block);

  DISALLOW_COPY_AND_ASSIGN(TryStatement);
};

// Represents 'using' statement:
//  'using' '(' ResouceDecl ')' EmbededStatement
class UsingStatement final : public SimpleNode<Statement, 2> {
  DECLARE_CONCRETE_AST_NODE_CLASS(UsingStatement, Statement);

 public:
  Expression* resource() const;
  Statement* statement() const;
  Variable* variable() const { return variable_; }

 private:
  UsingStatement(Token* keyword,
                 Variable* variable,
                 Expression* resource,
                 Statement* statement);

  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(UsingStatement);
};

// Represents variable declaration
//  VarDecl ::= Name '=' Expression
// Note: A |value| of a variable declared in 'for-each' statement contains
// invalid value. We can't use it.
class VarDeclaration final : public SimpleNode<NamedNode, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(VarDeclaration, NamedNode);

 public:
  Expression* expression() const;
  Variable* variable() const { return variable_; }
  Type* type() const;

 private:
  VarDeclaration(Token* token, Variable* variable, Expression* value);

  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(VarDeclaration);
};

// Represents 'var' statement:
//  'var' VarDecl (',' VarDecl)* ';'
class VarStatement final : public VariadicNode<Statement> {
  DECLARE_CONCRETE_AST_NODE_CLASS(VarStatement, Statement);

 public:
  ChildNodes<VarDeclaration> variables() const;

 private:
  // |type_token| comes from variable type node.
  VarStatement(Zone* zone,
               Token* type_token,
               const std::vector<VarDeclaration*>& variables);

  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

// Represents 'while' statement:
//  'while' '(' BooleanExpression ')' EmbededStatement
class WhileStatement final : public SimpleNode<Statement, 2> {
  DECLARE_CONCRETE_AST_NODE_CLASS(WhileStatement, Statement);

 public:
  Expression* condition() const;
  Statement* statement() const;

 private:
  WhileStatement(Token* keyword, Expression* condition, Statement* statement);

  DISALLOW_COPY_AND_ASSIGN(WhileStatement);
};

// Represents 'yield' statement:
//  'yield' Expression
class YieldStatement final : public SimpleNode<Statement, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(YieldStatement, Statement);

 public:
  Expression* expression() const;

 private:
  YieldStatement(Token* keyword, Expression* value);

  DISALLOW_COPY_AND_ASSIGN(YieldStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_STATEMENTS_H_
