// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "elang/compiler/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/block_statement.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/if_statement.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/method_group.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/ast/return_statement.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/ast/var_statement.h"
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
  for (;;) {
    // TODO(eval1749) Should we do unreachable code check?
    if (!ParseStatement()) {
      if (AdvanceIf(TokenType::RightCurryBracket))
        break;
      // We don't have right curry bracket. We reached at end of source.
      Error(ErrorCode::SyntaxStatementInvalid);
      Advance();
      continue;
    }
    statements.push_back(ConsumeStatement());
  }
  ProduceStatement(factory()->NewBlockStatement(bracket, statements));
  return true;
}

// IfStatement ::= 'if' '(' Expression ')' Statement (('else Statement ')')?
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

// Parses statement in following grammar:
//    BlockStatement
//    BreakStatement NYI
//    ContinueStatement NYI
//    DoStatement NYI
//    ExpressionStatement
//    ForStatement NYI
//    ForEachStatement NYI
//    GotoEachStatement NYI
//    IfStatement
//    ReturnStatement
//    SwitchStatement NYI
//    TryStatement NYI
//    UsingStatement NYI
//    WhileStatement NYI
//    YieldStatement NYI
bool Parser::ParseStatement() {
  if (auto const bracket = ConsumeTokenIf(TokenType::LeftCurryBracket))
    return ParseBlockStatement(bracket);

  if (auto const if_keyword = ConsumeTokenIf(TokenType::If))
    return ParseIfStatement(if_keyword);

  if (auto const return_keyword = ConsumeTokenIf(TokenType::Return))
    return ParseReturnStatement(return_keyword);

  // ExpressionStatement ::= Expression ';'
  if (!ParseExpression())
    return false;
  if (AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxStatementSemiColon);
  return true;
}

void Parser::ProduceStatement(ast::Statement* statement) {
  DCHECK(!statement_);
  statement_ = statement;
}

}  // namespace compiler
}  // namespace elang
