// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SYNTAX_PARSER_H_
#define ELANG_COMPILER_SYNTAX_PARSER_H_

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/compilation_session_user.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

class CompilationUnit;
enum class ErrorCode;
class Lexer;
class Modifiers;
class SourceCodePosition;

//////////////////////////////////////////////////////////////////////
//
// Parser
//
class Parser final : public CompilationSessionUser {
  // |ExpressionCategory| representing precedence of operator.
  // Note: To implement operation of |ExpressionCategory| in "parser.cc"
  // it is marked |public|, but other modules can't use it.
 public:
  enum class ExpressionCategory;

  Parser(CompilationSession* session, CompilationUnit* compilation_unit);
  ~Parser();

  // The entry point of |Parser|.
  void Run();

 private:
  class ContainerScope;
  class ModifierParser;

  ast::Factory* factory() const;

  void Advance();
  bool AdvanceIf(TokenType type);

  Token* ConsumeName(ErrorCode error_code);
  Token* ConsumeOperator(TokenType type, ErrorCode error_code);

  // If curren token isn't semi-conlon, report error with |error_code|.
  void ConsumeSemiColon(ErrorCode error_code);

  // Returns current token and advance to next token.
  Token* ConsumeToken();

  // Returns current token as |type| and advance to next token. This function
  // is used for handling unary '+','-' and post '++', '--'.
  Token* ConsumeTokenAs(TokenType type);

  // Returns |Token*| and consume it if current token is |type|.
  Token* ConsumeTokenIf(TokenType type);

  // Always returns false for simplify error processing.
  bool Error(ErrorCode error_code);
  bool Error(ErrorCode error_code, Token* token);
  bool Error(ErrorCode error_code, Token* token, Token* token2);
  Token* NewUniqueNameToken(const base::char16* format);

  bool ParseClass();
  void ParseEnum();
  void ParseFunction();
  void ParseField(Token* keyword, ast::Type* type, Token* name);
  void ParseCompilationUnit();
  bool ParseNamespace();
  bool ParseNamespace(Token* namespace_keyword,
                      const std::vector<Token*>& names,
                      size_t index);
  void ParseNamedNodes();
  void ParseUsingDirectives();
  Token* PeekToken();
  void ValidateClassModifiers();
  void ValidateEnumModifiers();
  void ValidateFieldModifiers();
  void ValidateMethodModifiers();

  // in "parse_expression.cc"
  // Returns last produced expression.
  ast::Expression* ConsumeExpression();
  ast::Expression* ConsumeExpressionOrType();
  ast::Expression* NewInvalidExpression(Token* token);
  void ParseExpression(ErrorCode error_code);
  bool ParseExpressionSub(ExpressionCategory category);
  bool ParsePrimaryExpression();
  void ParsePrimaryExpressionPost();
  void ParseTypeArguments();
  bool ParseUnaryExpression();
  ExpressionCategory PeekTokenCategory();
  void ProduceBinaryOperation(Token* op_token,
                              ast::Expression* left,
                              ast::Expression* right);
  void ProduceExpression(ast::Expression* expression);
  void ProduceExpressionOrType(ast::Expression* expression);
  void ProduceInvalidExpression(Token* token);
  void ProduceIncrementExpression(Token* op_token, ast::Expression* expression);
  void ProduceNameReference(Token* token);
  void ProduceUnaryOperation(Token* op_token, ast::Expression* expression);
  Token* TryConsumeUnaryOperator();
  bool TryParseExpression();

  // in "parse_statement.cc"
  // Returns last produced statement.
  class LocalDeclarationSpace;
  class StatementScope;
  ast::Statement* ConsumeStatement();
  ast::NamedNode* FindLocalMember(Token* token) const;
  bool IsBound(ast::Variable* variable) const;
  bool IsInLoop() const;
  bool IsInStatement(TokenType keyword) const;
  void ParseBlockStatement(Token* keyword);
  void ParseBreakStatement(Token* keyword);
  void ParseConstStatement(Token* keyword);
  void ParseContinueStatement(Token* keyword);
  void ParseDoStatement(Token* keyword);
  void ParseForStatement(Token* keyword);
  void ParseIfStatement(Token* keyword);
  void ParseMethod(Modifiers modifiers,
                   ast::Type* method_type,
                   Token* method_name,
                   const std::vector<Token*> type_parameters);
  void ParseReturnStatement(Token* keyword);
  void ParseThrowStatement(Token* keyword);
  void ParseTryStatement(Token* keyword);
  void ParseStatement();
  void ParseWhileStatement(Token* keyword);
  void ParseUsingStatement(Token* keyword);
  void ParseVarStatement(Token* keyword);
  // Returns name followed by optional type.
  Token* ParseVarTypeAndName();
  void ParseVariables(Token* keyword, ast::Type* type, Token* maybe_name);
  void ParseYieldStatement(Token* keyword);
  void ProduceStatement(ast::Statement* statement);

  // Produce local variable or parameter reference at |name| token.
  void ProduceVariableReference(Token* name, ast::NamedNode* variable);

  // in "parse_type.cc"
  // Returns last produced expression.
  ast::Type* ConsumeExpressionAsType();
  ast::Type* ConsumeType();
  // Returns true if |expression| can be type. Since we've not yet resolved
  // name references, |expression| may not be type.
  bool MaybeType(ast::Expression* expression) const;
  // Returns true if |expression| can be type name. Since we've not yet resolved
  // name references, |expression| may not be type name.
  bool MaybeTypeName(ast::Expression* expression) const;
  ast::Type* NewTypeNameReference(Token* token);
  ast::Type* ParseAndConsumeType();
  void ParseArrayType(Token* bracket);
  bool ParseNamespaceOrTypeName();
  void ParseType();
  void ParseTypeAfterName();
  std::vector<Token*> ParseTypeParameterList();
  void ParseTypePost();
  void ProduceInvalidType(Token* token);
  void ProduceType(ast::Type* type);
  void ProduceTypeNameReference(ast::NameReference* node);
  void ProduceTypeNameReference(Token* token);

  CompilationUnit* compilation_unit_;
  ast::BodyNode* container_;
  LocalDeclarationSpace* declaration_space_;
  std::vector<Token*> delimiters_;
  ast::Expression* expression_;
  const std::unique_ptr<Lexer> lexer_;
  int last_source_offset_;
  std::unique_ptr<ModifierParser> modifiers_;
  ast::Statement* statement_;
  StatementScope* statement_scope_;
  Token* token_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

class Parser::ContainerScope final {
 public:
  ContainerScope(Parser* parser, ast::BodyNode* new_container);
  ~ContainerScope();

 private:
  Parser* const parser_;
  ast::BodyNode* const container_;

  DISALLOW_COPY_AND_ASSIGN(ContainerScope);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SYNTAX_PARSER_H_
