// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <unordered_set>

#include "elang/compiler/syntax/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/parameter_kind.h"
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
  bool IsBound(ast::Variable* variable) const;
  void RecordBind(ast::Variable* variable);
  void RecordReference(ast::NamedNode* member);

 private:
  std::unordered_set<ast::Variable*> bound_variables_;
  LocalDeclarationSpace* outer_;
  Token* const owner_;
  Parser* const parser_;
  std::unordered_map<AtomicString*, ast::NamedNode*> map_;
  std::unordered_set<ast::NamedNode*> referenced_set_;

  DISALLOW_COPY_AND_ASSIGN(LocalDeclarationSpace);
};

Parser::LocalDeclarationSpace::LocalDeclarationSpace(Parser* parser,
                                                     Token* owner)
    : outer_(parser->declaration_space_), owner_(owner), parser_(parser) {
  parser_->declaration_space_ = this;
}

Parser::LocalDeclarationSpace::~LocalDeclarationSpace() {
  for (auto name_node : map_) {
    if (referenced_set_.count(name_node.second))
      continue;
    // TODO(eval1749) We should use SyntaxLabelNotUsed for labels.
    parser_->Error(ErrorCode::SyntaxVarNotUsed, name_node.second->name());
  }
  parser_->declaration_space_ = outer_;
}

void Parser::LocalDeclarationSpace::AddMember(ast::NamedNode* member) {
  auto const name = member->name()->atomic_string();
  if (map_.count(name))
    return;
  map_[name] = member;
}

ast::NamedNode* Parser::LocalDeclarationSpace::FindMember(Token* name) const {
  DCHECK(name->is_name()) << *name;
  auto const present = map_.find(name->atomic_string());
  return present == map_.end() ? nullptr : present->second;
}

bool Parser::LocalDeclarationSpace::IsBound(ast::Variable* variable) const {
  DCHECK(FindMember(variable->name())) << *variable;
  return bound_variables_.count(variable) != 0;
}

void Parser::LocalDeclarationSpace::RecordBind(ast::Variable* variable) {
  DCHECK(!IsBound(variable)) << *variable;
  bound_variables_.insert(variable);
}

void Parser::LocalDeclarationSpace::RecordReference(ast::NamedNode* member) {
  DCHECK(map_.count(member->name()->atomic_string()));
  referenced_set_.insert(member);
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

bool Parser::IsBound(ast::Variable* variable) const {
  auto const name = variable->name();
  for (auto space = declaration_space_; space; space = space->outer()) {
    if (auto const present = space->FindMember(name)) {
      if (present == variable)
        return space->IsBound(variable);
    }
  }
  NOTREACHED() << *variable;
  return false;
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
void Parser::ParseBlockStatement(Token* bracket) {
  DCHECK_EQ(bracket, TokenType::LeftCurryBracket);
  LocalDeclarationSpace block_space(this, bracket);
  std::vector<ast::Statement*> statements;
  auto reachable = true;
  auto right_bracket = static_cast<Token*>(nullptr);
  for (;;) {
    right_bracket = ConsumeTokenIf(TokenType::RightCurryBracket);
    if (right_bracket)
      break;
    // TODO(eval1749) We should |reachable| when we get labeled statement.
    ParseStatement();
    auto const statement = ConsumeStatement();
    if (!reachable)
      Error(ErrorCode::SyntaxStatementUnreachable, statement->token());
    statements.push_back(statement);
    if (reachable && statement->IsTerminator())
      reachable = false;
  }
  ProduceStatement(factory()->NewBlockStatement(
      reachable ? bracket : right_bracket, statements));
}

void Parser::ParseBreakStatement(Token* break_keyword) {
  DCHECK_EQ(break_keyword, TokenType::Break);
  ProduceStatement(factory()->NewBreakStatement(break_keyword));
  ConsumeSemiColon(ErrorCode::SyntaxBreakSemiColon);
  if (!IsInLoop())
    Error(ErrorCode::SyntaxBreakInvalid);
}

// ConstStatement ::= 'const' Type? VarDecl (',' VarDecl)*
// VarDecl ::= Name '=' Expression
void Parser::ParseConstStatement(Token* const_keyword) {
  DCHECK_EQ(const_keyword, TokenType::Const);
  auto const maybe_name = ParseVarTypeAndName();
  auto const type = ConsumeType();
  ParseVariables(const_keyword, type, maybe_name);
  ConsumeSemiColon(ErrorCode::SyntaxVarSemiColon);
}

void Parser::ParseContinueStatement(Token* continue_keyword) {
  DCHECK_EQ(continue_keyword, TokenType::Continue);
  ProduceStatement(factory()->NewContinueStatement(continue_keyword));
  ConsumeSemiColon(ErrorCode::SyntaxContinueSemiColon);
  if (!IsInLoop())
    Error(ErrorCode::SyntaxContinueInvalid);
}

// DoStatement ::= 'do' Statement 'while' '(' Expression ') ';'
void Parser::ParseDoStatement(Token* do_keyword) {
  DCHECK_EQ(do_keyword, TokenType::Do);
  StatementScope do_scope(this, do_keyword);
  ParseStatement();
  auto const statement = ConsumeStatement();
  if (!AdvanceIf(TokenType::While))
    Error(ErrorCode::SyntaxDoWhile);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxDoLeftParenthesis);
  ParseExpression(ErrorCode::SyntaxDoCondition);
  auto const condition = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxDoRightParenthesis);
  ConsumeSemiColon(ErrorCode::SyntaxDoSemiColon);
  ProduceStatement(factory()->NewDoStatement(do_keyword, statement, condition));
}

// ForThreeStatement ::=
//   'for' '(' ForInitializer? ';' ForCondition? ';' ForIterator ')'
//     EmbeddedStatement
// ForEachStatement ::=
//   'for' '(' ForEachInitializer ':' Expression ')'
//     EmbeddedStatement
void Parser::ParseForStatement(Token* for_keyword) {
  DCHECK_EQ(for_keyword, TokenType::For);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxForLeftParenthesis);

  enum class State {
    Body,
    Condition,
    ForEach,
    InitExpr,
    InitStart,
    InitVarDecl,
    InitVarDeclMore,
    Step,
  } state = State::InitStart;

  LocalDeclarationSpace for_var_scope(this, for_keyword);
  std::vector<ast::Expression*> initializers;
  std::vector<ast::Expression*> steps;
  std::vector<ast::VarDeclaration*> variables;
  ast::Expression* condition = nullptr;
  Token* semi_colon = nullptr;
  Token* var_keyword = nullptr;

  for (;;) {
    switch (state) {
      case State::Body: {
        if (initializers.empty() && variables.empty()) {
          if (!semi_colon) {
            semi_colon = session()->NewToken(for_keyword->location(),
                                             TokenData(TokenType::SemiColon));
          }
          ProduceStatement(factory()->NewEmptyStatement(semi_colon));
        } else if (initializers.empty()) {
          ProduceStatement(factory()->NewVarStatement(for_keyword, variables));
        } else if (variables.empty()) {
          ProduceStatement(
              factory()->NewExpressionList(for_keyword, initializers));
        } else {
          ProduceStatement(factory()->NewVarStatement(for_keyword, variables));
          Error(ErrorCode::SyntaxForInit);
        }
        auto const initializer = ConsumeStatement();
        auto const step =
            steps.empty() ? nullptr
                          : factory()->NewExpressionList(for_keyword, steps);
        StatementScope for_scope(this, for_keyword);
        ParseStatement();
        ProduceStatement(factory()->NewForStatement(
            for_keyword, initializer, condition, step, ConsumeStatement()));
        return;
      }

      case State::ForEach: {
        DCHECK(initializers.empty());
        DCHECK(!variables.empty());
        if (variables.size() != 1u)
          Error(ErrorCode::SyntaxForColon);
        ParseExpression(ErrorCode::SyntaxForColon);
        if (!AdvanceIf(TokenType::RightParenthesis))
          Error(ErrorCode::SyntaxForRightParenthesis);
        auto const enumerable = ConsumeExpression();
        StatementScope for_scope(this, for_keyword);
        ParseStatement();
        ProduceStatement(factory()->NewForEachStatement(
            for_keyword, variables.front()->variable(), enumerable,
            ConsumeStatement()));
        return;
      }

      case State::Condition:
        if (AdvanceIf(TokenType::RightParenthesis)) {
          // Case: for (init;)
          Error(ErrorCode::SyntaxForCondition);
          state = State::Body;
          continue;
        }
        if (AdvanceIf(TokenType::SemiColon)) {
          state = State::Step;
          continue;
        }
        ParseExpression(ErrorCode::SyntaxForCondition);
        condition = ConsumeExpression();
        ConsumeSemiColon(ErrorCode::SyntaxForSemiColon);
        state = State::Step;
        continue;

      case State::InitExpr:
        initializers.push_back(ConsumeExpression());
        if (auto const token = ConsumeTokenIf(TokenType::SemiColon)) {
          semi_colon = token;
          state = State::Condition;
          continue;
        }
        if (AdvanceIf(TokenType::RightParenthesis)) {
          Error(ErrorCode::SyntaxForInit);
          state = State::Step;
          continue;
        }
        if (!AdvanceIf(TokenType::Comma))
          Error(ErrorCode::SyntaxForInit);
        ParseExpression(ErrorCode::SyntaxForInit);
        continue;

      case State::InitStart:
        if (auto const token = ConsumeTokenIf(TokenType::Const)) {
          var_keyword = token;
          state = State::InitVarDecl;
          continue;
        }
        if (auto const token = ConsumeTokenIf(TokenType::Var)) {
          var_keyword = token;
          state = State::InitVarDecl;
          continue;
        }
        if (auto const token = ConsumeTokenIf(TokenType::SemiColon)) {
          semi_colon = token;
          state = State::Condition;
          continue;
        }
        if (AdvanceIf(TokenType::RightParenthesis)) {
          Error(ErrorCode::SyntaxForInit);
          state = State::Body;
          continue;
        }
        if (PeekToken()->is_type_name()) {
          var_keyword = session()->NewToken(
              PeekToken()->location(),
              TokenData(TokenType::Var, session()->NewAtomicString(L"var")));
          state = State::InitVarDecl;
          continue;
        }
        ParseExpression(ErrorCode::SyntaxForInit);
        state = State::InitExpr;
        continue;

      case State::InitVarDecl: {
        auto const name = ParseVarTypeAndName();
        auto const type = ConsumeType();
        auto const variable = factory()->NewVariable(var_keyword, type, name);
        declaration_space_->AddMember(variable);
        if (auto const colon = ConsumeTokenIf(TokenType::Colon)) {
          // Bind 'for-each' variable to dummy expression to avoid unbound
          // variable error.
          variables.push_back(factory()->NewVarDeclaration(
              colon, variable, NewInvalidExpression(colon)));
          declaration_space_->RecordBind(variable);
          state = State::ForEach;
          continue;
        }
        auto const assign =
            ConsumeOperator(TokenType::Assign, ErrorCode::SyntaxForInit);
        ParseExpression(ErrorCode::SyntaxForInit);
        auto const init = ConsumeExpression();
        variables.push_back(
            factory()->NewVarDeclaration(assign, variable, init));
        declaration_space_->RecordBind(variable);
        ProduceType(type);
        state = State::InitVarDeclMore;
        continue;
      }

      case State::InitVarDeclMore: {
        if (AdvanceIf(TokenType::SemiColon)) {
          ConsumeType();
          state = State::Condition;
          continue;
        }
        if (AdvanceIf(TokenType::RightParenthesis)) {
          // for (var ...)
          Error(ErrorCode::SyntaxForInit);
          ConsumeType();
          state = State::Body;
          continue;
        }
        if (!AdvanceIf(TokenType::Comma))
          Error(ErrorCode::SyntaxForInit);
        auto const type = ConsumeType();
        auto const name = ConsumeName(ErrorCode::SyntaxForInit);
        auto const variable = factory()->NewVariable(var_keyword, type, name);
        declaration_space_->AddMember(variable);
        auto const assign =
            ConsumeOperator(TokenType::Assign, ErrorCode::SyntaxForInit);
        ParseExpression(ErrorCode::SyntaxForInit);
        auto const init = ConsumeExpression();
        variables.push_back(
            factory()->NewVarDeclaration(assign, variable, init));
        declaration_space_->RecordBind(variable);
        ProduceType(type);
        continue;
      }

      case State::Step:
        if (AdvanceIf(TokenType::RightParenthesis)) {
          state = State::Body;
          continue;
        }
        ParseExpression(ErrorCode::SyntaxForStep);
        steps.push_back(ConsumeExpression());
        if (AdvanceIf(TokenType::Comma))
          continue;
        if (!AdvanceIf(TokenType::RightParenthesis))
          Error(ErrorCode::SyntaxForRightParenthesis);
        state = State::Body;
        continue;
    }
  }
}

// IfStatement ::= 'if' '(' Expression ')' Statement ('else Statement)?
void Parser::ParseIfStatement(Token* if_keyword) {
  DCHECK_EQ(if_keyword, TokenType::If);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxIfLeftParenthesis);
  ParseExpression(ErrorCode::SyntaxIfCondition);
  auto const condition = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxIfRightParenthesis);
  ParseStatement();
  auto const then_statement = ConsumeStatement();
  auto else_statement = static_cast<ast::Statement*>(nullptr);
  if (AdvanceIf(TokenType::Else)) {
    ParseStatement();
    else_statement = ConsumeStatement();
  }
  ProduceStatement(factory()->NewIfStatement(if_keyword, condition,
                                             then_statement, else_statement));
}

// Called after '(' read.
void Parser::ParseMethod(Modifiers method_modifiers,
                         ast::Type* method_type,
                         Token* method_name,
                         const std::vector<Token*> type_parameters) {
  ValidateMethodModifiers();

  auto const class_body = container_->as<ast::ClassBody>();
  auto const method = factory()->NewMethod(
      class_body, method_modifiers, method_type, method_name, type_parameters);
  container_->AddMember(method);
  auto const method_body = factory()->NewMethodBody(method);
  method->AddMember(method_body);

  ContainerScope container_scope(this, method_body);
  LocalDeclarationSpace method_space(this, PeekToken());
  if (!AdvanceIf(TokenType::RightParenthesis)) {
    std::vector<ast::Parameter*> parameters;
    auto position = 0;
    for (;;) {
      auto const param_type = ParseAndConsumeType();
      if (!PeekToken()->is_name())
        Error(ErrorCode::SyntaxMethodParameter);
      auto const param_name =
          PeekToken()->is_name() ? ConsumeToken() : NewUniqueNameToken(L"@p%d");
      if (method_space.FindMember(param_name))
        Error(ErrorCode::SyntaxMethodNameDuplicate);
      auto const parameter =
          factory()->NewParameter(method, ParameterKind::Required, position,
                                  param_type, param_name, nullptr);
      method_space.AddMember(parameter);
      parameters.push_back(parameter);
      if (method_modifiers.HasAbstract() || method_modifiers.HasExtern()) {
        // To avoid "not used" warning of parameter in abstract and extern
        // methods, since these methods don't have method body, we mark
        // parameter as used.
        method_space.RecordReference(parameter);
      }
      ++position;
      if (AdvanceIf(TokenType::RightParenthesis))
        break;
      if (!AdvanceIf(TokenType::Comma))
        Error(ErrorCode::SyntaxMethodComma);
    }
    method->SetParameters(parameters);
  }

  if (AdvanceIf(TokenType::SemiColon)) {
    if (!method_modifiers.HasAbstract() && !method_modifiers.HasExtern())
      Error(ErrorCode::SyntaxMethodSemiColon);
    return;
  }

  if (PeekToken() != TokenType::LeftCurryBracket) {
    Error(ErrorCode::SyntaxMethodLeftCurryBracket);
    return;
  }

  if (method_modifiers.HasAbstract() || method_modifiers.HasExtern())
    Error(ErrorCode::SyntaxMethodBody);

  LocalDeclarationSpace method_body_space(this, PeekToken());
  ParseStatement();
  method->SetBody(ConsumeStatement());
}

// ReturnStatement ::= 'return' Expression? ';'
void Parser::ParseReturnStatement(Token* return_keyword) {
  DCHECK_EQ(return_keyword, TokenType::Return);
  if (AdvanceIf(TokenType::SemiColon)) {
    return ProduceStatement(
        factory()->NewReturnStatement(return_keyword, nullptr));
  }
  ParseExpression(ErrorCode::SyntaxReturnExpression);
  auto const value = ConsumeExpression();
  ConsumeSemiColon(ErrorCode::SyntaxReturnSemiColon);
  ProduceStatement(factory()->NewReturnStatement(return_keyword, value));
}

// ThrowStatement ::= 'throw' Expression? ';'
// Note: We can omit Expression if throw-statement in catch-clause.
void Parser::ParseThrowStatement(Token* throw_keyword) {
  DCHECK_EQ(throw_keyword, TokenType::Throw);
  if (AdvanceIf(TokenType::SemiColon)) {
    if (!IsInStatement(TokenType::Catch))
      Error(ErrorCode::SyntaxThrowInvalid);
    ProduceStatement(factory()->NewThrowStatement(throw_keyword, nullptr));
    return;
  }

  ParseExpression(ErrorCode::SyntaxThrowExpression);
  ProduceStatement(
      factory()->NewThrowStatement(throw_keyword, ConsumeExpression()));
  ConsumeSemiColon(ErrorCode::SyntaxThrowSemiColon);
}

// TryStatement ::= 'try' Block CatchClause* ('finally' Block)?
void Parser::ParseTryStatement(Token* try_keyword) {
  DCHECK_EQ(try_keyword, TokenType::Try);
  if (PeekToken() != TokenType::LeftCurryBracket)
    Error(ErrorCode::SyntaxTryLeftCurryBracket);
  ParseStatement();
  auto const protected_block = ConsumeStatement()->as<ast::BlockStatement>();

  // Parser 'catch' '(' Type Name? ')' Block
  std::vector<ast::CatchClause*> catch_clauses;
  while (auto const catch_keyword = ConsumeTokenIf(TokenType::Catch)) {
    if (!AdvanceIf(TokenType::LeftParenthesis))
      Error(ErrorCode::SyntaxCatchLeftParenthesis);
    auto const catch_type = ParseAndConsumeType();
    auto catch_var = static_cast<ast::Variable*>(nullptr);
    StatementScope catch_scope(this, catch_keyword);
    LocalDeclarationSpace catch_var_scope(this, catch_keyword);
    if (PeekToken()->is_name()) {
      auto const catch_name = ConsumeToken();
      catch_var = factory()->NewVariable(catch_keyword, catch_type, catch_name);
      catch_var_scope.AddMember(catch_var);
    }
    if (!AdvanceIf(TokenType::RightParenthesis))
      Error(ErrorCode::SyntaxCatchRightParenthesis);
    auto left_bracket = ConsumeToken();
    if (left_bracket != TokenType::LeftCurryBracket) {
      Error(ErrorCode::SyntaxCatchLeftCurryBracket);
      left_bracket = session()->NewToken(
          left_bracket->location(), TokenData(TokenType::LeftCurryBracket));
    }
    ParseBlockStatement(left_bracket);
    auto const catch_block = ConsumeStatement()->as<ast::BlockStatement>();
    catch_clauses.push_back(factory()->NewCatchClause(catch_keyword, catch_type,
                                                      catch_var, catch_block));
  }

  // Parser 'finally' Block
  if (!AdvanceIf(TokenType::Finally)) {
    ProduceStatement(factory()->NewTryStatement(try_keyword, protected_block,
                                                catch_clauses, nullptr));
    return;
  }

  auto left_bracket = ConsumeToken();
  if (left_bracket != TokenType::LeftCurryBracket) {
    Error(ErrorCode::SyntaxCatchLeftCurryBracket);
    left_bracket = session()->NewToken(left_bracket->location(),
                                       TokenData(TokenType::LeftCurryBracket));
  }
  ParseBlockStatement(left_bracket);
  auto const finally_block = ConsumeStatement()->as<ast::BlockStatement>();
  ProduceStatement(factory()->NewTryStatement(try_keyword, protected_block,
                                              catch_clauses, finally_block));
}

// UsingStatement ::= 'using' '(' UsingResourceDecl ')' Statement
// UsingResourceDecl ::= Expression | 'var' Name '=' Expression
void Parser::ParseUsingStatement(Token* using_keyword) {
  DCHECK_EQ(using_keyword, TokenType::Using);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxUsingLeftParenthesis);
  if (auto const var_keyword = ConsumeTokenIf(TokenType::Var)) {
    if (!PeekToken()->is_name())
      Error(ErrorCode::SyntaxUsingName);
    auto const var_name = ConsumeToken();
    if (!AdvanceIf(TokenType::Assign))
      Error(ErrorCode::SyntaxUsingAssign);
    ParseExpression(ErrorCode::SyntaxUsingExpression);
    if (!AdvanceIf(TokenType::RightParenthesis))
      Error(ErrorCode::SyntaxUsingRightParenthesis);

    LocalDeclarationSpace using_scope(this, using_keyword);
    auto const var_type = factory()->NewTypeVariable(var_keyword);
    auto const variable =
        factory()->NewVariable(using_keyword, var_type, var_name);
    using_scope.AddMember(variable);
    auto const resource = ConsumeExpression();
    declaration_space_->RecordBind(variable);
    ParseStatement();
    ProduceStatement(factory()->NewUsingStatement(
        using_keyword, variable, resource, ConsumeStatement()));
    return;
  }

  ParseExpression(ErrorCode::SyntaxUsingExpression);
  auto const resource = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxUsingRightParenthesis);
  ParseStatement();
  ProduceStatement(factory()->NewUsingStatement(using_keyword, nullptr,
                                                resource, ConsumeStatement()));
}

// |keyword| is 'const', 'var' or token of |type|.
void Parser::ParseVariables(Token* keyword,
                            ast::Type* type,
                            Token* maybe_name) {
  std::vector<ast::VarDeclaration*> variables;
  for (;;) {
    if (!maybe_name->is_name()) {
      Error(ErrorCode::SyntaxVarName, maybe_name);
      if (maybe_name == TokenType::SemiColon)
        break;
      maybe_name =
          session()->NewUniqueNameToken(maybe_name->location(), L"@var%d");
    }
    auto const name = maybe_name;
    auto const assign = PeekToken() == TokenType::Assign
                            ? ConsumeToken()
                            : session()->NewToken(PeekToken()->location(),
                                                  TokenData(TokenType::Assign));
    if (assign != TokenType::Assign)
      Error(ErrorCode::SyntaxVarAssign);
    auto const variable = factory()->NewVariable(keyword, type, name);
    if (FindLocalMember(name))
      Error(ErrorCode::SyntaxVarDuplicate, name);
    else
      declaration_space_->AddMember(variable);
    ParseExpression(ErrorCode::SyntaxVarInitializer);
    auto const init = ConsumeExpression();
    variables.push_back(factory()->NewVarDeclaration(assign, variable, init));
    declaration_space_->RecordBind(variable);
    if (!AdvanceIf(TokenType::Comma))
      break;
    if (PeekToken() == TokenType::SemiColon) {
      Error(ErrorCode::SyntaxVarName);
      break;
    }
    maybe_name = ConsumeToken();
  }
  if (variables.empty())
    return ProduceStatement(factory()->NewInvalidStatement(keyword));
  ProduceStatement(factory()->NewVarStatement(keyword, variables));
}

// VarStatement ::= 'var' Type? VarDecl (',' VarDecl)* ';'
// VarDecl ::= Name ('=' Expression')
void Parser::ParseVarStatement(Token* var_keyword) {
  DCHECK_EQ(var_keyword, TokenType::Var);
  auto const maybe_name = ParseVarTypeAndName();
  auto const type = ConsumeType();
  ParseVariables(var_keyword, type, maybe_name);
  ConsumeSemiColon(ErrorCode::SyntaxVarSemiColon);
}

// WhileStatement ::= while' '(' Expression ') Statement
void Parser::ParseWhileStatement(Token* while_keyword) {
  DCHECK_EQ(while_keyword, TokenType::While);
  if (!AdvanceIf(TokenType::LeftParenthesis))
    Error(ErrorCode::SyntaxWhileLeftParenthesis);
  ParseExpression(ErrorCode::SyntaxWhileCondition);
  auto const condition = ConsumeExpression();
  if (!AdvanceIf(TokenType::RightParenthesis))
    Error(ErrorCode::SyntaxWhileRightParenthesis);
  StatementScope while_scope(this, while_keyword);
  ParseStatement();
  auto const statement = ConsumeStatement();
  ProduceStatement(
      factory()->NewWhileStatement(while_keyword, condition, statement));
}

// YieldStatement ::= 'yield' expression ';'
void Parser::ParseYieldStatement(Token* yield_keyword) {
  DCHECK_EQ(yield_keyword, TokenType::Yield);
  ParseExpression(ErrorCode::SyntaxYieldExpression);
  auto const value = ConsumeExpression();
  ProduceStatement(factory()->NewYieldStatement(yield_keyword, value));
  ConsumeSemiColon(ErrorCode::SyntaxStatementSemiColon);
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
void Parser::ParseStatement() {
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

  if (auto const semi_colon = ConsumeTokenIf(TokenType::SemiColon))
    return ProduceStatement(factory()->NewEmptyStatement(semi_colon));

  // ExpressionStatement ::= Expression ';'
  if (!TryParseExpression())
    return ProduceStatement(factory()->NewInvalidStatement(ConsumeToken()));

  if (PeekToken()->is_name()) {
    // VariableDeclration ::=
    //    Type VariableDeclarator (',' VariableDeclarator)*
    // VariableDeclarator ::= Name ('=' Expression)
    auto const type = ConsumeType();
    auto const var_keyword = session()->NewToken(
        type->token()->location(),
        TokenData(TokenType::Var, session()->NewAtomicString(L"var")));
    ParseVariables(var_keyword, type, ConsumeToken());
    ConsumeSemiColon(ErrorCode::SyntaxVarSemiColon);
    return;
  }

  // Expression statement
  ProduceStatement(factory()->NewExpressionStatement(ConsumeExpression()));
  ConsumeSemiColon(ErrorCode::SyntaxStatementSemiColon);
}

void Parser::ProduceStatement(ast::Statement* statement) {
  DCHECK(!statement_);
  statement_ = statement;
}

void Parser::ProduceVariableReference(Token* name, ast::NamedNode* thing) {
  DCHECK(name->is_name());
  auto enclosing_space = static_cast<LocalDeclarationSpace*>(nullptr);
  for (auto runner = declaration_space_; runner; runner = runner->outer()) {
    if (runner->FindMember(name)) {
      enclosing_space = runner;
      break;
    }
  }
  DCHECK(enclosing_space);
  enclosing_space->RecordReference(thing);
  if (auto const var = thing->as<ast::Variable>())
    return ProduceExpression(factory()->NewVariableReference(name, var));
  if (auto const param = thing->as<ast::Parameter>())
    return ProduceExpression(factory()->NewParameterReference(name, param));
  NOTREACHED();
  ProduceInvalidExpression(name);
}

}  // namespace compiler
}  // namespace elang
