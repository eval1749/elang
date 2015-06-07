// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/statements.h"

#include "base/logging.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/types.h"
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
  return ChildNodes<Statement>(this, 0);
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
                         Type* type,
                         Variable* variable,
                         BlockStatement* block)
    : SimpleNode(nullptr, keyword), variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Catch);
  set_child_at(0, type);
  set_child_at(1, block);
}

BlockStatement* CatchClause::block() const {
  return child_at(1)->as<BlockStatement>();
}

Type* CatchClause::type() const {
  return child_at(0)->as<Type>();
}

// ContinueStatement
ContinueStatement::ContinueStatement(Token* keyword)
    : TerminatorStatement(keyword) {
  DCHECK_EQ(keyword, TokenType::Continue);
}

// DoStatement
DoStatement::DoStatement(Token* keyword,
                         Statement* statement,
                         Expression* condition)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::Do);
  set_child_at(0, statement);
  set_child_at(1, condition);
}

Expression* DoStatement::condition() const {
  return child_at(1)->as<Expression>();
}

Statement* DoStatement::statement() const {
  return child_at(0)->as<Statement>();
}

// EmptyStatement
EmptyStatement::EmptyStatement(Token* keyword) : Statement(keyword) {
  DCHECK_EQ(keyword, TokenType::SemiColon);
}

ExpressionList::ExpressionList(Zone* zone,
                               Token* keyword,
                               const std::vector<Expression*>& expressions)
    : VariadicNode(zone, expressions, keyword) {
}

ChildNodes<Expression> ExpressionList::expressions() const {
  return ChildNodes<Expression>(this, 0);
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
    : SimpleNode(keyword), variable_(variable) {
  set_child_at(0, enumerable);
  set_child_at(1, statement);
}

Expression* ForEachStatement::enumerable() const {
  return child_at(0)->as<Expression>();
}

Statement* ForEachStatement::statement() const {
  return child_at(1)->as<Statement>();
}

// ForStatement
ForStatement::ForStatement(Token* keyword,
                           Statement* initializer,
                           Expression* condition,
                           Statement* step,
                           Statement* statement)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::For);
  set_child_at(0, initializer);
  set_child_at(1, condition);
  set_child_at(2, step);
  set_child_at(3, statement);
}

Statement* ForStatement::initializer() const {
  return child_at(0)->as<Statement>();
}

Expression* ForStatement::condition() const {
  auto const expression = child_at(1)->as<Expression>();
  return expression->is<NoExpression>() ? nullptr : expression;
}

Statement* ForStatement::step() const {
  auto const statement = child_at(2)->as<Statement>();
  return statement->is<NoStatement>() ? nullptr : statement;
}

Statement* ForStatement::statement() const {
  return child_at(3)->as<Statement>();
}

// IfStatement
IfStatement::IfStatement(Token* keyword,
                         Expression* condition,
                         Statement* then_statement,
                         Statement* else_statement)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::If);
  set_child_at(0, condition);
  set_child_at(1, then_statement);
  set_child_at(2, else_statement);
}

Expression* IfStatement::condition() const {
  return child_at(0)->as<Expression>();
}

Statement* IfStatement::else_statement() const {
  auto const statement = child_at(2)->as<Statement>();
  return statement->is<NoStatement>() ? nullptr : statement;
}

Statement* IfStatement::then_statement() const {
  return child_at(1)->as<Statement>();
}

// Node
bool IfStatement::IsTerminator() const {
  return else_statement() && then_statement()->IsTerminator() &&
         else_statement()->IsTerminator();
}

// InvalidStatement
InvalidStatement::InvalidStatement(Token* token) : Statement(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

// NoStatement
NoStatement::NoStatement(Token* token) : Statement(token) {
  // We should have non-null |token| for source code location.
  DCHECK(token);
}

// ReturnStatement
ReturnStatement::ReturnStatement(Token* keyword, Expression* value)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::Return);
  set_child_at(0, value);
}

ast::Expression* ReturnStatement::expression() const {
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

ast::Expression* ThrowStatement::expression() const {
  auto const value = child_at(0)->as<ast::Expression>();
  return value->is<ast::NoExpression>() ? nullptr : value;
}

// TryStatement
TryStatement::TryStatement(Zone* zone,
                           Token* keyword,
                           BlockStatement* protected_block,
                           const std::vector<CatchClause*>& catch_clauses,
                           Statement* finally_block)
    : VariadicNode(zone, std::vector<Node*>{}, keyword) {
  DCHECK_EQ(keyword, TokenType::Try);
  AppendChild(protected_block);
  AppendChild(finally_block);
  for (auto const catch_clause : catch_clauses)
    AppendChild(catch_clause);
}

ChildNodes<CatchClause> TryStatement::catch_clauses() const {
  return ChildNodes<CatchClause>(this, 2);
}

BlockStatement* TryStatement::finally_block() const {
  auto const statement = child_at(1)->as<BlockStatement>();
  return statement->is<NoStatement>() ? nullptr : statement;
}

BlockStatement* TryStatement::protected_block() const {
  return child_at(0)->as<BlockStatement>();
}

// UsingStatement
UsingStatement::UsingStatement(Token* keyword,
                               Variable* variable,
                               Expression* resource,
                               Statement* statement)
    : SimpleNode(keyword), variable_(variable) {
  DCHECK_EQ(keyword, TokenType::Using);
  set_child_at(0, resource);
  set_child_at(1, statement);
}

Expression* UsingStatement::resource() const {
  return child_at(0)->as<Expression>();
}

Statement* UsingStatement::statement() const {
  return child_at(1)->as<Statement>();
}

// VarDeclaration
VarDeclaration::VarDeclaration(Token* token,
                               Variable* variable,
                               Expression* expression)
    : SimpleNode(nullptr, token, variable->name()), variable_(variable) {
  DCHECK(token == TokenType::Assign || token == TokenType::Colon) << token;
  DCHECK(variable_);
  set_child_at(0, expression);
}

Type* VarDeclaration::type() const {
  return variable_->type();
}

Expression* VarDeclaration::expression() const {
  return child_at(0)->as<Expression>();
}

// VarStatement
VarStatement::VarStatement(Zone* zone,
                           Token* type_token,
                           const std::vector<VarDeclaration*>& variables)
    : VariadicNode(zone, variables, type_token) {
}

ChildNodes<VarDeclaration> VarStatement::variables() const {
  return ChildNodes<VarDeclaration>(this, 0);
}

// WhileStatement
WhileStatement::WhileStatement(Token* keyword,
                               Expression* condition,
                               Statement* statement)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::While);
  set_child_at(0, condition);
  set_child_at(1, statement);
}

Expression* WhileStatement::condition() const {
  return child_at(0)->as<Expression>();
}

Statement* WhileStatement::statement() const {
  return child_at(1)->as<Statement>();
}

// YieldStatement
YieldStatement::YieldStatement(Token* keyword, Expression* value)
    : SimpleNode(keyword) {
  DCHECK_EQ(keyword, TokenType::Yield);
  set_child_at(0, value);
}

ast::Expression* YieldStatement::expression() const {
  return child_at(0)->as<ast::Expression>();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
