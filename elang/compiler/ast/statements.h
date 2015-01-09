// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_STATEMENTS_H_
#define ELANG_COMPILER_AST_STATEMENTS_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Statement
//
class Statement : public Node {
  DECLARE_AST_NODE_CLASS(Statement, Node);

 public:
  Token* keyword() const { return token(); }

 protected:
  explicit Statement(Token* keyword);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

// Represents block statement:
//  '{' Statement* '}'
class BlockStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(BlockStatement, Statement);

 public:
  const ZoneVector<Statement*>& statements() const { return statements_; }

 private:
  BlockStatement(Zone* zone,
                 Token* keyword,
                 const std::vector<Statement*>& statements);

  const ZoneVector<Statement*> statements_;

  DISALLOW_COPY_AND_ASSIGN(BlockStatement);
};

// Represents 'break' statement
//  'break' ';'
class BreakStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(BreakStatement, Statement);

 private:
  explicit BreakStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(BreakStatement);
};

// Represents 'catch' clause:
//  'catch' '(' Type Name? ')' '(' Statement* '}'
class CatchClause final : public Node {
  DECLARE_AST_NODE_CLASS(CatchClause, Node);

 public:
  BlockStatement* block() const { return block_; }
  Expression* type() const { return type_; }
  LocalVariable* variable() const { return variable_; }

 private:
  CatchClause(Token* keyword,
              Expression* type,
              LocalVariable* variable,
              BlockStatement* block);

  BlockStatement* const block_;
  Expression* const type_;
  LocalVariable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(CatchClause);
};

// Represents 'continue' statement
//  'continue' ';'
class ContinueStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ContinueStatement, Statement);

 private:
  explicit ContinueStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(ContinueStatement);
};

// Represents 'do' statement:
//   'do' EmbededStatement 'while' '(' BooleanExpression ')' ';'
class DoStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(DoStatement, Statement);

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
  DECLARE_AST_NODE_CONCRETE_CLASS(EmptyStatement, Statement);

 private:
  explicit EmptyStatement(Token* keyword);

  DISALLOW_COPY_AND_ASSIGN(EmptyStatement);
};

// Represents comma separated expressions used in 'for' statement.
class ExpressionList : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ExpressionList, Statement);

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
  DECLARE_AST_NODE_CONCRETE_CLASS(ExpressionStatement, Statement);

 public:
  Expression* expression() const { return expression_; }

 private:
  explicit ExpressionStatement(Expression* expression);

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionStatement);
};

// Represents 'for' + ':' statement.
class ForEachStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ForEachStatement, Statement);

 public:
  Expression* enumerable() const { return enumerable_; }
  Statement* statement() const { return statement_; }
  LocalVariable* variable() const { return variable_; }

 private:
  ForEachStatement(Token* keyword,
                   LocalVariable* variable,
                   Expression* enumerable,
                   Statement* statement);

  Expression* const enumerable_;
  Statement* const statement_;
  LocalVariable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(ForEachStatement);
};

// Represents 'for' statement.
class ForStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ForStatement, Statement);

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
  DECLARE_AST_NODE_CONCRETE_CLASS(IfStatement, Statement);

 public:
  Expression* condition() const { return condition_; }
  Statement* else_statement() const { return else_statement_; }
  Statement* then_statement() const { return then_statement_; }

 private:
  IfStatement(Token* keyword,
              Expression* condition,
              Statement* then_statement,
              Statement* else_statement);

  Expression* const condition_;
  Statement* const else_statement_;
  Statement* const then_statement_;

  DISALLOW_COPY_AND_ASSIGN(IfStatement);
};

// Represents invalid statement. This statement is used for continuing parsing
// after syntax error.
class InvalidStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(InvalidStatement, Statement);

 private:
  explicit InvalidStatement(Token* token);

  DISALLOW_COPY_AND_ASSIGN(InvalidStatement);
};

// Represents 'return' statement:
//  'return' Expression? ';'
class ReturnStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ReturnStatement, Statement);

 public:
  Expression* value() const { return value_; }

 private:
  ReturnStatement(Token* keyword, Expression* value);

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(ReturnStatement);
};

// Represents 'throw' statement:
//  'throw' Expression
class ThrowStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ThrowStatement, Statement);

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
  DECLARE_AST_NODE_CONCRETE_CLASS(TryStatement, Statement);

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
  DECLARE_AST_NODE_CONCRETE_CLASS(UsingStatement, Statement);

 public:
  Expression* resource() const { return resource_; }
  Statement* statement() const { return statement_; }
  LocalVariable* variable() const { return variable_; }

 private:
  UsingStatement(Token* keyword,
                 LocalVariable* variable,
                 Expression* resource,
                 Statement* statement);

  Expression* const resource_;
  Statement* const statement_;
  LocalVariable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(UsingStatement);
};

// Represents 'var' statement:
//  'var' VarDecl (',' VarDecl)* ';'
//  VarDecl ::= Name ('=' Expression)
class VarStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(VarStatement, Statement);

 public:
  const ZoneVector<LocalVariable*>& variables() const { return variables_; }

 private:
  // |type_token| comes from variable type node.
  VarStatement(Zone* zone,
               Token* type_token,
               const std::vector<LocalVariable*>& variables);

  const ZoneVector<LocalVariable*> variables_;

  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

// Represents 'while' statement:
//  'while' '(' BooleanExpression ')' EmbededStatement
class WhileStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(WhileStatement, Statement);

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
  DECLARE_AST_NODE_CONCRETE_CLASS(YieldStatement, Statement);

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