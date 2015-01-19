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

  // Returns true if this statement is terminator, e.g. 'break', 'continue',
  // 'return', etc. 'if'-statement can be terminator if both then and
  // else clauses are terminator. Note: parser doesn't do constant expression
  // evaluation, so "while (true) {...}" isn't terminator.
  virtual bool IsTerminator() const;

 protected:
  explicit Statement(Token* keyword);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

// TerminatorStatement represents a statement which moves control other than
// following statement.
class TerminatorStatement : public Statement {
 protected:
  explicit TerminatorStatement(Token* keyword);
  ~TerminatorStatement() override;

 private:
  // Statement
  bool IsTerminator() const override;

  DISALLOW_COPY_AND_ASSIGN(TerminatorStatement);
};

// Represents block statement:
//  '{' Statement* '}'
class BlockStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(BlockStatement, Statement);

 public:
  const ZoneVector<Statement*>& statements() const { return statements_; }

 private:
  // When last statement of this block statement is reachable, |keyword|
  // is left curry bracket, otherwise it is right curry bracket.
  BlockStatement(Zone* zone,
                 Token* keyword,
                 const std::vector<Statement*>& statements);

  // Statement
  bool IsTerminator() const override;

  const ZoneVector<Statement*> statements_;

  DISALLOW_COPY_AND_ASSIGN(BlockStatement);
};

// Represents 'break' statement
//  'break' ';'
class BreakStatement final : public TerminatorStatement {
  DECLARE_CONCRETE_AST_NODE_CLASS(BreakStatement, TerminatorStatement);

 private:
  explicit BreakStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(BreakStatement);
};

// Represents 'catch' clause:
//  'catch' '(' Type Name? ')' '(' Statement* '}'
class CatchClause final : public Node {
  DECLARE_CONCRETE_AST_NODE_CLASS(CatchClause, Node);

 public:
  BlockStatement* block() const { return block_; }
  Expression* type() const { return type_; }
  Variable* variable() const { return variable_; }

 private:
  CatchClause(Token* keyword,
              Expression* type,
              Variable* variable,
              BlockStatement* block);

  BlockStatement* const block_;
  Expression* const type_;
  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(CatchClause);
};

// Represents 'continue' statement
//  'continue' ';'
class ContinueStatement final : public TerminatorStatement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ContinueStatement, TerminatorStatement);

 private:
  explicit ContinueStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(ContinueStatement);
};

// Represents 'do' statement:
//   'do' EmbededStatement 'while' '(' BooleanExpression ')' ';'
class DoStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(DoStatement, Statement);

 public:
  Expression* condition() const { return condition_; }
  Statement* statement() const { return statement_; }

 private:
  DoStatement(Token* keyword, Statement* statement, Expression* condition);

  Expression* const condition_;
  Statement* const statement_;

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
class ExpressionList : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ExpressionList, Statement);

 public:
  const std::vector<Expression*>& expressions() const { return expressions_; }

 protected:
  ExpressionList(Token* keyword, const std::vector<Expression*>& expressions);

 private:
  const std::vector<Expression*> expressions_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionList);
};

// Represents expression statement:
//  Expression ';'
class ExpressionStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ExpressionStatement, Statement);

 public:
  Expression* expression() const { return expression_; }

 private:
  explicit ExpressionStatement(Expression* expression);

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

// Represents 'for' + ':' statement.
class ForEachStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ForEachStatement, Statement);

 public:
  Expression* enumerable() const { return enumerable_; }
  Statement* statement() const { return statement_; }
  Variable* variable() const { return variable_; }

 private:
  ForEachStatement(Token* keyword,
                   Variable* variable,
                   Expression* enumerable,
                   Statement* statement);

  Expression* const enumerable_;
  Statement* const statement_;
  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(ForEachStatement);
};

// Represents 'for' statement.
class ForStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ForStatement, Statement);

 public:
  Expression* condition() const { return condition_; }
  Statement* initializer() const { return initializer_; }
  Statement* statement() const { return statement_; }
  Statement* step() const { return step_; }

 private:
  ForStatement(Token* keyword,
               Statement* initializer,
               Expression* condition,
               Statement* step,
               Statement* statement);

  Expression* const condition_;
  Statement* const initializer_;
  Statement* const statement_;
  Statement* const step_;

  DISALLOW_COPY_AND_ASSIGN(ForStatement);
};

// Represents 'if' statement:
//  'if' '(' BooleanExpression ')' EmbededStatement ('else' EmbededStatement)?
class IfStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(IfStatement, Statement);

 public:
  Expression* condition() const { return condition_; }
  Statement* else_statement() const { return else_statement_; }
  Statement* then_statement() const { return then_statement_; }

 private:
  IfStatement(Token* keyword,
              Expression* condition,
              Statement* then_statement,
              Statement* else_statement);

  // Statement
  bool IsTerminator() const override;

  Expression* const condition_;
  Statement* const else_statement_;
  Statement* const then_statement_;

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

// Represents 'return' statement:
//  'return' Expression? ';'
class ReturnStatement final : public TerminatorStatement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ReturnStatement, TerminatorStatement);

 public:
  Expression* value() const { return value_; }

 private:
  ReturnStatement(Token* keyword, Expression* value);

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(ReturnStatement);
};

// Represents 'throw' statement:
//  'throw' Expression
class ThrowStatement final : public TerminatorStatement {
  DECLARE_CONCRETE_AST_NODE_CLASS(ThrowStatement, TerminatorStatement);

 public:
  Expression* value() const { return value_; }

 private:
  ThrowStatement(Token* keyword, Expression* value);

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(ThrowStatement);
};

// Represents 'try' statement:
//  'try' BlockStatement (CatchCaluse*) ('finally' BlockStatement)?
class TryStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(TryStatement, Statement);

 public:
  BlockStatement* finally_block() const { return finally_block_; }
  const ZoneVector<CatchClause*>& catch_clauses() const {
    return catch_clauses_;
  }
  BlockStatement* protected_block() const { return protected_block_; }

 private:
  TryStatement(Zone* zone,
               Token* keyword,
               BlockStatement* protected_block,
               const std::vector<CatchClause*>& catch_clauses,
               BlockStatement* finally_block);

  const ZoneVector<CatchClause*> catch_clauses_;
  BlockStatement* const finally_block_;
  BlockStatement* const protected_block_;

  DISALLOW_COPY_AND_ASSIGN(TryStatement);
};

// Represents 'using' statement:
//  'using' '(' ResouceDecl ')' EmbededStatement
class UsingStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(UsingStatement, Statement);

 public:
  Expression* resource() const { return resource_; }
  Statement* statement() const { return statement_; }
  Variable* variable() const { return variable_; }

 private:
  UsingStatement(Token* keyword,
                 Variable* variable,
                 Expression* resource,
                 Statement* statement);

  Expression* const resource_;
  Statement* const statement_;
  Variable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(UsingStatement);
};

// Represents 'var' statement:
//  'var' VarDecl (',' VarDecl)* ';'
//  VarDecl ::= Name ('=' Expression)
class VarStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(VarStatement, Statement);

 public:
  const ZoneVector<Variable*>& variables() const { return variables_; }

 private:
  // |type_token| comes from variable type node.
  VarStatement(Zone* zone,
               Token* type_token,
               const std::vector<Variable*>& variables);

  const ZoneVector<Variable*> variables_;

  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

// Represents 'while' statement:
//  'while' '(' BooleanExpression ')' EmbededStatement
class WhileStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(WhileStatement, Statement);

 public:
  Expression* condition() const { return condition_; }
  Statement* statement() const { return statement_; }

 private:
  WhileStatement(Token* keyword, Expression* condition, Statement* statement);

  Expression* const condition_;
  Statement* const statement_;

  DISALLOW_COPY_AND_ASSIGN(WhileStatement);
};

// Represents 'yield' statement:
//  'yield' Expression
class YieldStatement final : public Statement {
  DECLARE_CONCRETE_AST_NODE_CLASS(YieldStatement, Statement);

 public:
  Expression* value() const { return value_; }

 private:
  YieldStatement(Token* keyword, Expression* value);

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(YieldStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_STATEMENTS_H_
