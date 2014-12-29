// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "elang/compiler/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/block_statement.h"
#include "elang/compiler/ast/break_statement.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/continue_statement.h"
#include "elang/compiler/ast/do_statement.h"
#include "elang/compiler/ast/empty_statement.h"
#include "elang/compiler/ast/expression_statement.h"
#include "elang/compiler/ast/if_statement.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/method_group.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/ast/return_statement.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/ast/var_statement.h"
#include "elang/compiler/ast/while_statement.h"
#include "elang/compiler/ast/yield_statement.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Parser::LocalDeclarationSpace
//
class Parser::LocalDeclarationSpace final {
 public:
  explicit LocalDeclarationSpace(Parser* parser, Token* owner);
  ~LocalDeclarationSpace();

  LocalDeclarationSpace* outer() const { return outer_; }
  Token* owner() const { return owner_; }

  void AddVarStatement(ast::VarStatement* variable);
  ast::VarStatement* FindVariable(Token* name) const;

 private:
  LocalDeclarationSpace* outer_;
  Token* const owner_;
  Parser* const parser_;
  std::unordered_map<hir::SimpleName*, ast::VarStatement*> variables_;

  DISALLOW_COPY_AND_ASSIGN(LocalDeclarationSpace);
};

Parser::LocalDeclarationSpace::LocalDeclarationSpace(Parser* parser,
                                                     Token* owner)
    : outer_(parser->declaration_space_), owner_(owner), parser_(parser) {
  parser_->declaration_space_ = this;
}

Parser::LocalDeclarationSpace::~LocalDeclarationSpace() {
  parser_->declaration_space_ = outer_;
}

void Parser::LocalDeclarationSpace::AddVarStatement(
    ast::VarStatement* variable) {
  auto const name = variable->name()->simple_name();
  if (variables_.find(name) != variables_.end())
    return;
  variables_[name] = variable;
}

ast::VarStatement* Parser::LocalDeclarationSpace::FindVariable(
    Token* name) const {
  DCHECK(name->is_name());
  auto const present = variables_.find(name->simple_name());
  return present == variables_.end() ? nullptr : present->second;
}

//////////////////////////////////////////////////////////////////////
//
// Parser::StatementScope
//
class Parser::StatementScope final {
 public:
  StatementScope(Parser* parser, Token* keyword);
  ~StatementScope();

  StatementScope* outer() const { return outer_; }

  bool IsLoop() const;
  bool IsSwitch() const;

 private:
  Token* const keyword_;
  StatementScope* outer_;
  Parser* const parser_;

  DISALLOW_COPY_AND_ASSIGN(StatementScope);
};

Parser::StatementScope::StatementScope(Parser* parser, Token* keyword)
    : keyword_(keyword), outer_(parser->statement_scope_), parser_(parser) {
  parser_->statement_scope_ = this;
}

Parser::StatementScope::~StatementScope() {
  parser_->statement_scope_ = outer_;
}

bool Parser::StatementScope::IsLoop() const {
  return keyword_ == TokenType::Do || keyword_ == TokenType::For ||
         keyword_ == TokenType::While;
}

bool Parser::StatementScope::IsSwitch() const {
  return keyword_ == TokenType::Switch;
}

//////////////////////////////////////////////////////////////////////
//
// Parser
//
ast::Statement* Parser::ConsumeStatement() {
  DCHECK(statement_);
  auto const result = statement_;
  statement_ = nullptr;
  return result;
}

ast::VarStatement* Parser::FindVariable(Token* token) const {
  DCHECK(token->is_name());
  for (auto space = declaration_space_; space; space = space->outer()) {
    if (auto const present = space->FindVariable(token))
      return present;
  }
  return nullptr;
}

// BlockStatement ::= '{' Statement* '}'
bool Parser::ParseBlockStatement(Token* bracket) {
  DCHECK_EQ(bracket, TokenType::LeftCurryBracket);
  LocalDeclarationSpace block_space(this, bracket);
  std::vector<ast::Statement*> statements;
  while (!AdvanceIf(TokenType::RightCurryBracket)) {
    // TODO(eval1749) Should we do unreachable code check?
    if (!ParseStatement())
      break;
    statements.push_back(ConsumeStatement());
  }
  ProduceStatement(factory()->NewBlockStatement(bracket, statements));
  return true;
}

bool Parser::ParseBreakStatement(Token* break_keyword) {
  DCHECK_EQ(break_keyword, TokenType::Break);
  ProduceStatement(factory()->NewBreakStatement(break_keyword));
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxBreakSemiColon);
  for (auto scope = statement_scope_; scope; scope = scope->outer()) {
    if (scope->IsLoop() || scope->IsSwitch())
      return true;
  }
  Error(ErrorCode::SyntaxBreakInvalid);
  return true;
}

bool Parser::ParseContinueStatement(Token* continue_keyword) {
  DCHECK_EQ(continue_keyword, TokenType::Continue);
  ProduceStatement(factory()->NewContinueStatement(continue_keyword));
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxContinueSemiColon);
  for (auto scope = statement_scope_; scope; scope = scope->outer()) {
    if (scope->IsLoop())
      return true;
  }
  Error(ErrorCode::SyntaxContinueInvalid);
  return true;
}

// DoStatement ::= 'do' Statement 'while' '(' Expression ') ';'
bool Parser::ParseDoStatement(Token* do_keyword) {
  DCHECK_EQ(do_keyword, TokenType::Do);
  StatementScope do_scope(this, do_keyword);
  if (!ParseStatement())
    return false;
  auto const statement = ConsumeStatement();
  if (!AdvanceIf(TokenType::While)) {
    Error(ErrorCode::SyntaxDoWhile);
    return false;
  }
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxDoLeftParenthesis);
  if (!ParseExpression())
    return false;
  auto const condition = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxDoRightParenthesis);
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxDoSemiColon);
  ProduceStatement(factory()->NewDoStatement(do_keyword, statement, condition));
  return true;
}

// IfStatement ::= 'if' '(' Expression ')' Statement ('else Statement)?
bool Parser::ParseIfStatement(Token* if_keyword) {
  DCHECK_EQ(if_keyword, TokenType::If);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxIfLeftParenthesis);
  if (!ParseExpression())
    return false;
  auto const condition = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxIfRightParenthesis);
  if (!ParseStatement())
    return false;
  auto const then_statement = ConsumeStatement();
  auto else_statement = static_cast<ast::Statement*>(nullptr);
  if (AdvanceIf(TokenType::Else)) {
    if (ParseStatement())
      else_statement = ConsumeStatement();
  }
  ProduceStatement(factory()->NewIfStatement(if_keyword, condition,
                                             then_statement, else_statement));
  return true;
}

// Called after '(' read.
bool Parser::ParseMethodDecl(Modifiers method_modifiers,
                             ast::Expression* method_type,
                             Token* method_name,
                             const std::vector<Token*> type_parameters) {
  ValidateMethodModifiers();
  std::vector<ast::VarStatement*> parameters;
  std::unordered_set<hir::SimpleName*> names;
  for (;;) {
    auto const param_type = ParseType() ? ConsumeType() : nullptr;
    auto const param_name =
        PeekToken()->is_name() ? ConsumeToken() : NewUniqueNameToken(L"@p%d");
    if (names.find(param_name->simple_name()) != names.end())
      Error(ErrorCode::SyntaxMethodNameDuplicate);
    parameters.push_back(
        factory()->NewVarStatement(param_type, param_name, nullptr));
    names.insert(param_name->simple_name());
    if (AdvanceIf(TokenType::RightParenthesis))
      break;
    if (!AdvanceIf(TokenType::Comma))
      Error(ErrorCode::SyntaxMethodComma);
  }

  auto method_group = static_cast<ast::MethodGroup*>(nullptr);
  if (auto const present = FindMember(method_name)) {
    method_group = present->as<ast::MethodGroup>();
    if (!method_group)
      Error(ErrorCode::SyntaxClassMemberDuplicate, method_name);
  }
  if (!method_group) {
    method_group = factory()->NewMethodGroup(namespace_body_, method_name);
    AddMember(method_group);
  }

  auto const method = factory()->NewMethod(
      namespace_body_, method_group, method_modifiers, method_type, method_name,
      type_parameters, parameters);
  method_group->AddMethod(method);

  if (AdvanceIf(TokenType::SemiColon)) {
    if (!method_modifiers.HasExtern())
      Error(ErrorCode::SyntaxMethodSemiColon);
    return true;
  }

  if (PeekToken() != TokenType::LeftCurryBracket) {
    Error(ErrorCode::SyntaxMethodLeftCurryBracket);
    return true;
  }

  LocalDeclarationSpace method_body_space(this, PeekToken());
  for (auto param : method->parameters())
    method_body_space.AddVarStatement(param);

  if (!ParseStatement())
    return true;

  auto const method_body = ConsumeStatement();
  DCHECK(method_body->is<ast::BlockStatement>());
  method->SetStatement(method_body);
  return true;
}

// ReturnStatement ::= 'return' expression? ';'
bool Parser::ParseReturnStatement(Token* return_keyword) {
  DCHECK_EQ(return_keyword, TokenType::Return);
  auto value = static_cast<ast::Expression*>(nullptr);
  if (!AdvanceIf(TokenType::SemiColon)) {
    if (ParseExpression()) {
      value = ConsumeExpression();
      if (!AdvanceIf(TokenType::SemiColon))
        Error(ErrorCode::SyntaxStatementSemiColon);
    }
  }
  ProduceStatement(factory()->NewReturnStatement(return_keyword, value));
  return true;
}

// WhileStatement ::= while' '(' Expression ') Statement
bool Parser::ParseWhileStatement(Token* while_keyword) {
  DCHECK_EQ(while_keyword, TokenType::While);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxWhileLeftParenthesis);
  if (!ParseExpression())
    return false;
  auto const condition = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxWhileRightParenthesis);
  StatementScope while_scope(this, while_keyword);
  if (!ParseStatement())
    return false;
  auto const statement = ConsumeStatement();
  ProduceStatement(
      factory()->NewWhileStatement(while_keyword, condition, statement));
  return true;
}

// YieldStatement ::= 'yield' expression ';'
bool Parser::ParseYieldStatement(Token* yield_keyword) {
  DCHECK_EQ(yield_keyword, TokenType::Yield);
  if (!ParseExpression())
    return false;
  auto const value = ConsumeExpression();
  ProduceStatement(factory()->NewYieldStatement(yield_keyword, value));
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxStatementSemiColon);
  return true;
}

// Parses statement in following grammar:
//    BlockStatement
//    BreakStatement
//    ContinueStatement
//    DoStatement
//    EmptyStatement
//    ExpressionStatement
//    ForStatement NYI
//    ForEachStatement NYI
//    GotoEachStatement NYI
//    IfStatement
//    ReturnStatement
//    SwitchStatement NYI
//    TryStatement NYI
//    UsingStatement NYI
//    VarStatement NYI
//    WhileStatement
//    YieldStatement
bool Parser::ParseStatement() {
  if (auto const bracket = ConsumeTokenIf(TokenType::LeftCurryBracket))
    return ParseBlockStatement(bracket);

  if (auto const break_keyword = ConsumeTokenIf(TokenType::Break))
    return ParseBreakStatement(break_keyword);

  if (auto const continue_keyword = ConsumeTokenIf(TokenType::Continue))
    return ParseContinueStatement(continue_keyword);

  if (auto const do_keyword = ConsumeTokenIf(TokenType::Do))
    return ParseDoStatement(do_keyword);

  if (auto const if_keyword = ConsumeTokenIf(TokenType::If))
    return ParseIfStatement(if_keyword);

  if (auto const return_keyword = ConsumeTokenIf(TokenType::Return))
    return ParseReturnStatement(return_keyword);

  if (auto const while_keyword = ConsumeTokenIf(TokenType::While))
    return ParseWhileStatement(while_keyword);

  if (auto const yield_keyword = ConsumeTokenIf(TokenType::Yield))
    return ParseYieldStatement(yield_keyword);

  if (auto const semicolon = ConsumeTokenIf(TokenType::SemiColon)) {
    ProduceStatement(factory()->NewEmptyStatement(semicolon));
    return true;
  }

  // ExpressionStatement ::= Expression ';'
  if (!ParseExpression())
    return false;
  ProduceStatement(factory()->NewExpressionStatement(ConsumeExpression()));
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxStatementSemiColon);
  return true;
}

void Parser::ProduceStatement(ast::Statement* statement) {
  DCHECK(!statement_);
  statement_ = statement;
}

}  // namespace compiler
}  // namespace elang
