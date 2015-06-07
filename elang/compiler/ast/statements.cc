// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/statements.h"

#include "base/logging.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

// BlockStatement
BlockStatement::BlockStatement(Zone* zone,
                               Token* keyword,
                               const std::vector<Statement*>& statements)
    : VariadicNode(zone, statements, keyword) {
  DCHECK(keyword == TokenType::LeftCurryBracket ||
         keyword == TokenType::RightCurryBracket);
}

ChildNodes<Statement> BlockStatement::statements() const {
  return ChildNodes<Statement>(this);
}

// Statement
bool BlockStatement::IsTerminator() const {
  return token() == TokenType::RightCurryBracket;
}

// BreakStatement
BreakStatement::BreakStatement(Token* keyword) : TerminatorStatement(keyword) {
  DCHECK_EQ(keyword, TokenType::Break);
}

CatchClause::CatchClause(Token* keyword,
                         Expression* type,
                         Variable* variable,
                         BlockStatement* block)
    : Node(nullptr, keyword), block_(block), type_(type), variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Catch);
  DCHECK(block_);
}

// ContinueStatement
ContinueStatement::ContinueStatement(Token* keyword)
    : TerminatorStatement(keyword) {
  DCHECK_EQ(keyword, TokenType::Continue);
}

DoOrWhileStatement::DoOrWhileStatement(Token* keyword,
                                       Statement* statement,
                                       Expression* condition)
    : Statement(keyword), condition_(condition), statement_(statement) {
  DCHECK(keyword == TokenType::Do || keyword == TokenType::For ||
         keyword == TokenType::While);
}

DoStatement::DoStatement(Token* keyword,
                         Statement* statement,
                         Expression* condition)
    : DoOrWhileStatement(keyword, statement, condition) {
  DCHECK_EQ(keyword, TokenType::Do);
}

EmptyStatement::EmptyStatement(Token* keyword) : Statement(keyword) {
  DCHECK_EQ(keyword, TokenType::SemiColon);
}

ExpressionList::ExpressionList(Token* keyword,
                               const std::vector<Expression*>& expressions)
    : Statement(keyword), expressions_(expressions) {
}

// ExpressionStatement
ExpressionStatement::ExpressionStatement(Expression* expression)
    : SimpleNode(expression->token()) {
  set_child_at(0, expression);
}

ast::Expression* ExpressionStatement::expression() const {
  return child_at(0)->as<ast::Expression>();
}

// ForEachStatement
ForEachStatement::ForEachStatement(Token* keyword,
                                   Variable* variable,
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
    : DoOrWhileStatement(keyword, statement, condition),
      initializer_(initializer),
      step_(step) {
  DCHECK_EQ(keyword, TokenType::For);
}

// IfStatement
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

bool IfStatement::IsTerminator() const {
  return else_statement_ && then_statement_->IsTerminator() &&
         else_statement_->IsTerminator();
}

// InvalidStatement
InvalidStatement::InvalidStatement(Token* token) : Statement(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

// ReturnStatement
ReturnStatement::ReturnStatement(Token* keyword, Expression* value)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::Return);
  set_child_at(0, value);
}

ast::Expression* ReturnStatement::value() const {
  auto const value = child_at(0)->as<ast::Expression>();
  return value->is<ast::NoExpression>() ? nullptr : value;
}

// Statement
Statement::Statement(Token* op) : Node(nullptr, op) {
}

bool Statement::IsTerminator() const {
  return false;
}

// TerminatorStatement
TerminatorStatement::TerminatorStatement(Token* keyword) : Statement(keyword) {
}

TerminatorStatement::~TerminatorStatement() {
}

// Statement
bool TerminatorStatement::IsTerminator() const {
  return true;
}

// ThrowStatement
ThrowStatement::ThrowStatement(Token* keyword, Expression* value)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::Throw);
  set_child_at(0, value);
}

ast::Expression* ThrowStatement::value() const {
  auto const value = child_at(0)->as<ast::Expression>();
  return value->is<ast::NoExpression>() ? nullptr : value;
}

// TryStatement
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
                               Variable* variable,
                               Expression* resource,
                               Statement* statement)
    : Statement(keyword),
      resource_(resource),
      statement_(statement),
      variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Using);
  DCHECK(resource_);
}

VarDeclaration::VarDeclaration(Token* token,
                               Variable* variable,
                               Expression* value)
    : NamedNode(nullptr, token, variable->name()),
      value_(value),
      variable_(variable) {
  DCHECK(token == TokenType::Assign || token == TokenType::Colon) << token;
  DCHECK(value_);
  DCHECK(variable_);
}

Type* VarDeclaration::type() const {
  return variable_->type();
}

VarStatement::VarStatement(Zone* zone,
                           Token* type_token,
                           const std::vector<VarDeclaration*>& variables)
    : Statement(type_token), variables_(zone, variables) {
}

WhileStatement::WhileStatement(Token* keyword,
                               Expression* condition,
                               Statement* statement)
    : DoOrWhileStatement(keyword, statement, condition) {
  DCHECK_EQ(keyword, TokenType::While);
}

YieldStatement::YieldStatement(Token* keyword, Expression* value)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::Yield);
  set_child_at(0, value);
}

ast::Expression* YieldStatement::value() const {
  return child_at(0)->as<ast::Expression>();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
