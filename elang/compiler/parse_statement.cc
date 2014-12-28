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
// Parser::DeclarationSpace
//
class Parser::DeclarationSpace final {
 public:
  explicit DeclarationSpace(Parser* parser, Token* owner);
  ~DeclarationSpace();

  DeclarationSpace* outer() const { return outer_; }
  Token* owner() const { return owner_; }

  void AddVarStatement(ast::VarStatement* variable);
  ast::VarStatement* FindVariable(Token* name) const;

 private:
  DeclarationSpace* outer_;
  Token* const owner_;
  Parser* const parser_;
  std::unordered_map<hir::SimpleName*, ast::VarStatement*> variables_;

  DISALLOW_COPY_AND_ASSIGN(DeclarationSpace);
};

Parser::DeclarationSpace::DeclarationSpace(Parser* parser, Token* owner)
    : outer_(parser->declaration_space_), owner_(owner), parser_(parser) {
  parser_->declaration_space_ = this;
}

Parser::DeclarationSpace::~DeclarationSpace() {
  parser_->declaration_space_ = outer_;
}

void Parser::DeclarationSpace::AddVarStatement(ast::VarStatement* variable) {
  auto const name = variable->name()->simple_name();
  if (variables_.find(name) != variables_.end())
    return;
  variables_[name] = variable;
}

ast::VarStatement* Parser::DeclarationSpace::FindVariable(Token* name) const {
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

  DeclarationSpace method_body_space(this, PeekToken());
  for (auto param : method->parameters())
    method_body_space.AddVarStatement(param);

  if (!ParseStatement())
    return true;

  method->SetStatement(ConsumeStatement());
  return true;
}

// Parses statement in following grammar:
//    BlockStatement
//    ExpressionStatement
//    ReturnStatement
bool Parser::ParseStatement() {
  if (auto const bracket = ConsumeTokenIf(TokenType::LeftCurryBracket)) {
    DeclarationSpace block_space(this, bracket);
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

  // ReturnStatement ::= 'return' expression? ';'
  if (auto const return_keyword = ConsumeTokenIf(TokenType::Return)) {
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
