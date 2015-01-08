// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "elang/compiler/syntax/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/block_statement.h"
#include "elang/compiler/ast/break_statement.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/catch_clause.h"
#include "elang/compiler/ast/continue_statement.h"
#include "elang/compiler/ast/do_statement.h"
#include "elang/compiler/ast/empty_statement.h"
#include "elang/compiler/ast/expression.h"
#include "elang/compiler/ast/expression_statement.h"
#include "elang/compiler/ast/if_statement.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/local_variable.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/method_group.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/ast/return_statement.h"
#include "elang/compiler/ast/throw_statement.h"
#include "elang/compiler/ast/try_statement.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/ast/using_statement.h"
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
  LocalDeclarationSpace(Parser* parser, Token* owner);
  ~LocalDeclarationSpace();

  LocalDeclarationSpace* outer() const { return outer_; }
  Token* owner() const { return owner_; }

  void AddMember(ast::NamedNode* member);
  ast::NamedNode* FindMember(Token* name) const;

 private:
  LocalDeclarationSpace* outer_;
  Token* const owner_;
  Parser* const parser_;
  std::unordered_map<AtomicString*, ast::NamedNode*> map_;

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

void Parser::LocalDeclarationSpace::AddMember(ast::NamedNode* member) {
  auto const name = member->name()->simple_name();
  if (map_.find(name) != map_.end())
    return;
  map_[name] = member;
}

ast::NamedNode* Parser::LocalDeclarationSpace::FindMember(Token* name) const {
  DCHECK(name->is_name());
  auto const present = map_.find(name->simple_name());
  return present == map_.end() ? nullptr : present->second;
}

//////////////////////////////////////////////////////////////////////
//
// Parser::StatementScope
//
class Parser::StatementScope final {
 public:
  StatementScope(Parser* parser, Token* keyword);
  ~StatementScope();

  Token* keyword() const { return keyword_; }
  StatementScope* outer() const { return outer_; }

  bool IsLoop() const;

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

ast::NamedNode* Parser::FindLocalMember(Token* token) const {
  DCHECK(token->is_name());
  for (auto space = declaration_space_; space; space = space->outer()) {
    if (auto const present = space->FindMember(token))
      return present;
  }
  return nullptr;
}

bool Parser::IsInLoop() const {
  for (auto scope = statement_scope_; scope; scope = scope->outer()) {
    if (scope->IsLoop())
      return true;
  }
  return false;
}

bool Parser::IsInStatement(TokenType keyword) const {
  for (auto scope = statement_scope_; scope; scope = scope->outer()) {
    if (scope->keyword() == keyword)
      return true;
  }
  return false;
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
  if (!IsInLoop())
    Error(ErrorCode::SyntaxBreakInvalid);
  return true;
}

// ConstStatement ::= 'const' ('var' | Type) (Name '=' Expression)+ ';'
bool Parser::ParseConstStatement(Token* const_keyword) {
  DCHECK_EQ(const_keyword, TokenType::Const);
  auto type = static_cast<ast::Expression*>(nullptr);
  if (auto const var_keyword = ConsumeTokenIf(TokenType::Var)) {
    type = factory()->NewNameReference(var_keyword);
  } else if (ParseType()) {
    type = ConsumeType();
  }
  std::vector<ast::LocalVariable*> variables;
  while (PeekToken()->is_name()) {
    auto const name = ConsumeToken();
    if (AdvanceIf(TokenType::Assign) && ParseExpression()) {
      auto const expression = ConsumeExpression();
      variables.push_back(
          factory()->NewLocalVariable(const_keyword, type, name, expression));
    }
    if (!AdvanceIf(TokenType::Comma))
      break;
  }
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxVarSemiColon);
  if (variables.empty())
    Error(ErrorCode::SyntaxVarInvalid);
  ProduceStatement(factory()->NewVarStatement(const_keyword, variables));
  return true;
}

bool Parser::ParseContinueStatement(Token* continue_keyword) {
  DCHECK_EQ(continue_keyword, TokenType::Continue);
  ProduceStatement(factory()->NewContinueStatement(continue_keyword));
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxContinueSemiColon);
  if (!IsInLoop())
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

// ForThreeStatement ::=
//   'for' '(' ForInitializer? ';' ForCondition? ';' ForIterator ')'
//     EmbeddedStatement
// ForEachStatement ::=
//   'for' '(' ForEachInitializer ':' Expression ')'
//     EmbeddedStatement
bool Parser::ParseForStatement(Token* for_keyword) {
  DCHECK_EQ(for_keyword, TokenType::For);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxForLeftParenthesis);

  enum class State {
    Colon,
    Comma,
    Initializer,
    SemiColon,
    Start,
    Type,
    TypeOrExpression,
  } state = State::Start;

  std::vector<ast::Expression*> initializers;
  std::vector<ast::LocalVariable*> variables;

  for (;;) {
    switch (state) {
      case State::Colon: {
        auto const colon = ConsumeToken();
        DCHECK_EQ(colon, TokenType::Colon);
        DCHECK(initializers.empty());
        DCHECK(!variables.empty());
        if (variables.size() != 1u)
          Error(ErrorCode::SyntaxForColon);
        if (!ParseExpression())
          ProduceExpression(factory()->NewInvalidExpression(colon));
        if (!AdvanceIf(TokenType::RightParenthesis))
          Error(ErrorCode::SyntaxForRightParenthesis);
        auto const enumerable = ConsumeExpression();
        StatementScope for_scope(this, for_keyword);
        if (!ParseStatement())
          ProduceStatement(factory()->NewInvalidStatement(colon));
        ProduceStatement(factory()->NewForEachStatement(
            for_keyword, variables.front(), enumerable, ConsumeStatement()));
        return true;
      }

      case State::Comma:
        if (ParseExpression())
          state = State::Initializer;
        continue;

      case State::Initializer:
        initializers.push_back(ConsumeExpression());
        if (PeekToken() == TokenType::SemiColon) {
          state = State::SemiColon;
          continue;
        }
        if (!AdvanceIf(TokenType::Comma))
          Error(ErrorCode::SyntaxForInit);
        continue;

      case State::SemiColon: {
        auto const semi_colon = ConsumeToken();
        DCHECK_EQ(semi_colon, TokenType::SemiColon);
        if (initializers.empty() && variables.empty()) {
          ProduceStatement(factory()->NewEmptyStatement(semi_colon));
        } else if (initializers.empty()) {
          ProduceStatement(factory()->NewVarStatement(for_keyword, variables));
        } else if (variables.empty()) {
          ProduceStatement(
              factory()->NewExpressionList(semi_colon, initializers));
        } else {
          ProduceStatement(factory()->NewVarStatement(for_keyword, variables));
          Error(ErrorCode::SyntaxForInit);
        }
        auto const initializer = ConsumeStatement();
        auto condition =
            PeekToken() != TokenType::SemiColon && ParseExpression()
                ? ConsumeExpression()
                : static_cast<ast::Expression*>(nullptr);
        if (!AdvanceIf(TokenType::SemiColon))
          Error(ErrorCode::SyntaxForSemiColon);
        std::vector<ast::Expression*> steps;
        if (PeekToken() != TokenType::RightParenthesis) {
          while (ParseExpression()) {
            steps.push_back(ConsumeExpression());
            if (!AdvanceIf(TokenType::Comma))
              break;
          }
        }
        if (!AdvanceIf(TokenType::RightParenthesis))
          Error(ErrorCode::SyntaxForRightParenthesis);
        auto const step =
            steps.empty() ? nullptr
                          : factory()->NewExpressionList(for_keyword, steps);
        StatementScope for_scope(this, for_keyword);
        if (!ParseStatement())
          ProduceStatement(factory()->NewInvalidStatement(semi_colon));
        ProduceStatement(factory()->NewForStatement(
            for_keyword, initializer, condition, step, ConsumeStatement()));
        return true;
      }

      case State::Start:
        if (PeekToken() == TokenType::SemiColon) {
          state = State::SemiColon;
          continue;
        }
        if (PeekToken() == TokenType::Var) {
          ProduceType(factory()->NewNameReference(ConsumeToken()));
          state = State::Type;
          continue;
        }
        if (!ParseExpression())
          ProduceExpression(factory()->NewInvalidExpression(for_keyword));
        state = State::TypeOrExpression;
        break;

      case State::Type: {
        if (!PeekToken()->is_name()) {
          Error(ErrorCode::SyntaxForVar);
          state = State::Start;
          continue;
        }
        auto const type = ConsumeType();
        auto const name = ConsumeToken();
        auto const init = AdvanceIf(TokenType::Assign) && ParseExpression()
                              ? ConsumeExpression()
                              : static_cast<ast::Expression*>(nullptr);

        variables.push_back(
            factory()->NewLocalVariable(for_keyword, type, name, init));

        if (PeekToken() == TokenType::Colon) {
          if (init)
            Error(ErrorCode::SyntaxForColon);
          state = State::Colon;
          continue;
        }

        if (PeekToken() == TokenType::SemiColon) {
          state = State::SemiColon;
          continue;
        }

        if (AdvanceIf(TokenType::Comma)) {
          ProduceType(type);
          continue;
        }

        Error(ErrorCode::SyntaxForInit);
        continue;
      }

      case State::TypeOrExpression:
        state = PeekToken()->is_name() ? State::Type : State::Initializer;
        continue;
    }
  }
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
  std::vector<ast::LocalVariable*> parameters;
  std::unordered_set<AtomicString*> names;
  if (!AdvanceIf(TokenType::RightParenthesis)) {
    for (;;) {
      auto const param_type = ParseType() ? ConsumeType() : nullptr;
      auto const param_name =
          PeekToken()->is_name() ? ConsumeToken() : NewUniqueNameToken(L"@p%d");
      if (names.find(param_name->simple_name()) != names.end())
        Error(ErrorCode::SyntaxMethodNameDuplicate);
      parameters.push_back(factory()->NewLocalVariable(nullptr, param_type,
                                                       param_name, nullptr));
      names.insert(param_name->simple_name());
      if (AdvanceIf(TokenType::RightParenthesis))
        break;
      if (!AdvanceIf(TokenType::Comma))
        Error(ErrorCode::SyntaxMethodComma);
    }
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
    method_body_space.AddMember(param);

  if (!ParseStatement())
    return true;

  auto const method_body = ConsumeStatement();
  DCHECK(method_body->is<ast::BlockStatement>());
  method->SetStatement(method_body);
  return true;
}

// ReturnStatement ::= 'return' Expression? ';'
bool Parser::ParseReturnStatement(Token* return_keyword) {
  DCHECK_EQ(return_keyword, TokenType::Return);
  auto value = static_cast<ast::Expression*>(nullptr);
  if (!AdvanceIf(TokenType::SemiColon)) {
    if (ParseExpression()) {
      value = ConsumeExpression();
      if (!AdvanceIf(TokenType::SemiColon))
        Error(ErrorCode::SyntaxReturnSemiColon);
    }
  }
  ProduceStatement(factory()->NewReturnStatement(return_keyword, value));
  return true;
}

// ThrowStatement ::= 'throw' Expression? ';'
// Note: We can omit Expression if throw-statement in catch-clause.
bool Parser::ParseThrowStatement(Token* throw_keyword) {
  DCHECK_EQ(throw_keyword, TokenType::Throw);
  auto value = static_cast<ast::Expression*>(nullptr);
  if (AdvanceIf(TokenType::SemiColon)) {
    if (!IsInStatement(TokenType::Catch))
      Error(ErrorCode::SyntaxThrowInvalid);
  } else {
    if (ParseExpression()) {
      value = ConsumeExpression();
      if (!AdvanceIf(TokenType::SemiColon))
        Error(ErrorCode::SyntaxThrowSemiColon);
    }
  }
  ProduceStatement(factory()->NewThrowStatement(throw_keyword, value));
  return true;
}

// TryStatement ::= 'try' Block CatchClause* ('finally' Block)?
bool Parser::ParseTryStatement(Token* try_keyword) {
  DCHECK_EQ(try_keyword, TokenType::Try);
  if (PeekToken() != TokenType::LeftCurryBracket) {
    Error(ErrorCode::SyntaxTryLeftCurryBracket);
    return false;
  }
  if (!ParseStatement())
    return false;

  auto const protected_block = ConsumeStatement()->as<ast::BlockStatement>();

  // Parser 'catch' '(' Type Name? ')' Block
  std::vector<ast::CatchClause*> catch_clauses;
  while (auto const catch_keyword = ConsumeTokenIf(TokenType::Catch)) {
    if (!AdvanceIf(TokenType::LeftParenthesis))
      Error(ErrorCode::SyntaxCatchLeftParenthesis);
    if (!ParseType())
      continue;
    auto const catch_type = ConsumeType();
    auto catch_var = static_cast<ast::LocalVariable*>(nullptr);
    StatementScope catch_scope(this, catch_keyword);
    LocalDeclarationSpace catch_var_scope(this, catch_keyword);
    if (PeekToken()->is_name()) {
      auto const catch_name = ConsumeToken();
      catch_var = factory()->NewLocalVariable(catch_keyword, catch_type,
                                              catch_name, nullptr);
      catch_var_scope.AddMember(catch_var);
    }
    if (!AdvanceIf(TokenType::RightParenthesis))
      Error(ErrorCode::SyntaxCatchRightParenthesis);
    if (PeekToken() != TokenType::LeftCurryBracket)
      Error(ErrorCode::SyntaxCatchLeftCurryBracket);
    if (!ParseStatement())
      continue;
    auto const catch_block = ConsumeStatement()->as<ast::BlockStatement>();
    catch_clauses.push_back(factory()->NewCatchClause(catch_keyword, catch_type,
                                                      catch_var, catch_block));
  }

  // Parser 'finally' Block
  auto finally_block = static_cast<ast::BlockStatement*>(nullptr);
  if (AdvanceIf(TokenType::Finally)) {
    if (PeekToken() != TokenType::LeftCurryBracket)
      Error(ErrorCode::SyntaxFinallyLeftCurryBracket);
    if (ParseStatement())
      finally_block = ConsumeStatement()->as<ast::BlockStatement>();
  }
  ProduceStatement(factory()->NewTryStatement(try_keyword, protected_block,
                                              catch_clauses, finally_block));
  return true;
}

// UsingStatement ::= 'using' '(' UsingResourceDecl ')' Statement
// UsingResourceDecl ::= Expression | 'var' Name '=' Expression
bool Parser::ParseUsingStatement(Token* using_keyword) {
  DCHECK_EQ(using_keyword, TokenType::Using);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxUsingLeftParenthesis);
  if (auto const var_keyword = ConsumeTokenIf(TokenType::Var)) {
    if (!PeekToken()->is_name()) {
      Error(ErrorCode::SyntaxUsingName);
      return false;
    }
    auto const var_name = ConsumeToken();
    if (!AdvanceIf(TokenType::Assign)) {
      Error(ErrorCode::SyntaxUsingAssign);
      return false;
    }
    if (!ParseExpression())
      return false;
    if (!AdvanceIf(TokenType::RightParenthesis))
      Error(ErrorCode::SyntaxUsingRightParenthesis);

    LocalDeclarationSpace using_scope(this, using_keyword);
    auto const variable = factory()->NewLocalVariable(
        using_keyword, nullptr, var_name, ConsumeExpression());
    using_scope.AddMember(variable);
    if (!ParseStatement())
      return false;
    ProduceStatement(factory()->NewUsingStatement(
        using_keyword, variable, variable->value(), ConsumeStatement()));
    return true;
  }

  if (!ParseExpression())
    return false;
  auto const resource = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxUsingRightParenthesis);
  if (!ParseStatement())
    return false;
  ProduceStatement(factory()->NewUsingStatement(using_keyword, nullptr,
                                                resource, ConsumeStatement()));
  return true;
}

// VarStatement ::= 'var' VarDecl (',' VarDecl)* ';'
// VarDecl ::= Name ('=' Expression')
bool Parser::ParseVarStatement(Token* var_keyword) {
  DCHECK_EQ(var_keyword, TokenType::Var);
  auto const type = factory()->NewNameReference(var_keyword);
  std::vector<ast::LocalVariable*> variables;
  while (PeekToken()->is_name()) {
    auto const name = ConsumeToken();
    auto expression = static_cast<ast::Expression*>(nullptr);
    if (AdvanceIf(TokenType::Assign)) {
      if (ParseExpression())
        expression = ConsumeExpression();
    }
    variables.push_back(
        factory()->NewLocalVariable(nullptr, type, name, expression));
    if (!AdvanceIf(TokenType::Comma))
      break;
  }
  if (!AdvanceIf(TokenType::SemiColon))
    Error(ErrorCode::SyntaxVarSemiColon);
  if (variables.empty())
    Error(ErrorCode::SyntaxVarInvalid);
  ProduceStatement(factory()->NewVarStatement(var_keyword, variables));
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
//    ConstStatement
//    DoStatement
//    EmptyStatement
//    ExpressionStatement
//    ForStatement NYI
//    ForEachStatement NYI
//    GotoEachStatement NYI
//    IfStatement
//    LabeledStatement NYI
//    ReturnStatement
//    SwitchStatement NYI
//    ThrowStatement
//    TryStatement
//    UsingStatement
//    VarStatement
//    WhileStatement
//    YieldStatement
bool Parser::ParseStatement() {
  if (auto const bracket = ConsumeTokenIf(TokenType::LeftCurryBracket))
    return ParseBlockStatement(bracket);

  if (auto const break_keyword = ConsumeTokenIf(TokenType::Break))
    return ParseBreakStatement(break_keyword);

  if (auto const const_keyword = ConsumeTokenIf(TokenType::Const))
    return ParseConstStatement(const_keyword);

  if (auto const continue_keyword = ConsumeTokenIf(TokenType::Continue))
    return ParseContinueStatement(continue_keyword);

  if (auto const do_keyword = ConsumeTokenIf(TokenType::Do))
    return ParseDoStatement(do_keyword);

  if (auto const for_keyword = ConsumeTokenIf(TokenType::For))
    return ParseForStatement(for_keyword);

  if (auto const if_keyword = ConsumeTokenIf(TokenType::If))
    return ParseIfStatement(if_keyword);

  if (auto const return_keyword = ConsumeTokenIf(TokenType::Return))
    return ParseReturnStatement(return_keyword);

  if (auto const throw_keyword = ConsumeTokenIf(TokenType::Throw))
    return ParseThrowStatement(throw_keyword);

  if (auto const try_keyword = ConsumeTokenIf(TokenType::Try))
    return ParseTryStatement(try_keyword);

  if (auto const using_keyword = ConsumeTokenIf(TokenType::Using))
    return ParseUsingStatement(using_keyword);

  if (auto const var_keyword = ConsumeTokenIf(TokenType::Var))
    return ParseVarStatement(var_keyword);

  if (auto const while_keyword = ConsumeTokenIf(TokenType::While))
    return ParseWhileStatement(while_keyword);

  if (auto const yield_keyword = ConsumeTokenIf(TokenType::Yield))
    return ParseYieldStatement(yield_keyword);

  if (auto const semi_colon = ConsumeTokenIf(TokenType::SemiColon)) {
    ProduceStatement(factory()->NewEmptyStatement(semi_colon));
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

ast::Statement* Parser::ProduceStatement(ast::Statement* statement) {
  DCHECK(!statement_);
  return statement_ = statement;
}

}  // namespace compiler
}  // namespace elang
