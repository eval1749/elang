// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/statements.h"

#include "base/logging.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/local_variable.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Statement
//
Statement::Statement(Token* op) : Node(nullptr, op) {
}

BlockStatement::BlockStatement(Zone* zone,
                               Token* keyword,
                               const std::vector<Statement*>& statements)
    : Statement(keyword), statements_(zone, statements) {
  DCHECK_EQ(keyword, TokenType::LeftCurryBracket);
}

BreakStatement::BreakStatement(Token* keyword) : Statement(keyword) {
  DCHECK_EQ(keyword, TokenType::Break);
}

CatchClause::CatchClause(Token* keyword,
                         Expression* type,
                         LocalVariable* variable,
                         BlockStatement* block)
    : Node(nullptr, keyword), block_(block), type_(type), variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Catch);
  DCHECK(block_);
}

ContinueStatement::ContinueStatement(Token* keyword) : Statement(keyword) {
  DCHECK_EQ(keyword, TokenType::Continue);
}

DoStatement::DoStatement(Token* keyword,
                         Statement* statement,
                         Expression* condition)
    : Statement(keyword), condition_(condition), statement_(statement) {
  DCHECK_EQ(keyword, TokenType::Do);
}

EmptyStatement::EmptyStatement(Token* keyword) : Statement(keyword) {
  DCHECK_EQ(keyword, TokenType::SemiColon);
}

ExpressionList::ExpressionList(Token* keyword,
                               const std::vector<Expression*>& expressions)
    : Statement(keyword), expressions_(expressions) {
}

ExpressionStatement::ExpressionStatement(Expression* expression)
    : Statement(expression->token()), expression_(expression) {
}

ForEachStatement::ForEachStatement(Token* keyword,
                                   LocalVariable* variable,
                                   Expression* enumerable,
                                   Statement* statement)
    : Statement(keyword),
      enumerable_(enumerable),
      statement_(statement),
      variable_(variable) {
}

ForStatement::ForStatement(Token* keyword,
                           Statement* initializer,
                           Expression* condition,
                           Statement* step,
                           Statement* statement)
    : Statement(keyword),
      condition_(condition),
      initializer_(initializer),
      statement_(statement),
      step_(step) {
}

IfStatement::IfStatement(Token* keyword,
                         Expression* condition,
                         Statement* then_statement,
                         Statement* else_statement)
    : Statement(keyword),
      condition_(condition),
      else_statement_(else_statement),
      then_statement_(then_statement) {
  DCHECK_EQ(keyword, TokenType::If);
}

InvalidStatement::InvalidStatement(Token* token) : Statement(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

ReturnStatement::ReturnStatement(Token* keyword, Expression* value)
    : Statement(keyword), value_(value) {
  DCHECK_EQ(keyword, TokenType::Return);
}

ThrowStatement::ThrowStatement(Token* keyword, Expression* value)
    : Statement(keyword), value_(value) {
  DCHECK_EQ(keyword, TokenType::Throw);
}

TryStatement::TryStatement(Zone* zone,
                           Token* keyword,
                           BlockStatement* protected_block,
                           const std::vector<CatchClause*>& catch_clauses,
                           BlockStatement* finally_block)
    : Statement(keyword),
      catch_clauses_(zone, catch_clauses),
      finally_block_(finally_block),
      protected_block_(protected_block) {
  DCHECK_EQ(keyword, TokenType::Try);
}

UsingStatement::UsingStatement(Token* keyword,
                               LocalVariable* variable,
                               Expression* resource,
                               Statement* statement)
    : Statement(keyword),
      resource_(resource),
      statement_(statement),
      variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Using);
  DCHECK(!variable_ || variable_->value() == resource_);
}

VarStatement::VarStatement(Zone* zone,
                           Token* type_token,
                           const std::vector<LocalVariable*>& variables)
    : Statement(type_token), variables_(zone, variables) {
}

WhileStatement::WhileStatement(Token* keyword,
                               Expression* condition,
                               Statement* statement)
    : Statement(keyword), condition_(condition), statement_(statement) {
  DCHECK_EQ(keyword, TokenType::While);
}

YieldStatement::YieldStatement(Token* keyword, Expression* value)
    : Statement(keyword), value_(value) {
  DCHECK_EQ(keyword, TokenType::Yield);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
