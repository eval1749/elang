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
  bool ParseCompilationUnit();
  bool ParseNamespace();
  bool ParseNamespace(Token* namespace_keyword,
                      const std::vector<Token*>& names,
                      size_t index);
  bool ParseNamedNodes();
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
  bool ParseExpression();
  bool ParseExpressionSub(ExpressionCategory category);
  bool ParsePrimaryExpression();
  void ParsePrimaryExpressionPost();
  void ParseTypeArguments();
  bool ParseUnaryExpression();
  ExpressionCategory PeekTokenCategory();
  ast::Expression* ProduceBinaryOperation(Token* op_token,
                                          ast::Expression* left,
                                          ast::Expression* right);
  ast::Expression* ProduceExpression(ast::Expression* expression);
  ast::Expression* ProduceExpressionOrType(ast::Expression* expression);
  ast::Expression* ProduceIncrementExpression(Token* op_token,
                                              ast::Expression* expression);
  ast::Expression* ProduceNameReference(Token* token);
  ast::Expression* ProduceUnaryOperation(Token* op_token,
                                         ast::Expression* expression);
  Token* TryConsumeUnaryOperator();

  // in "parse_statement.cc"
  // Returns last produced statement.
  class LocalDeclarationSpace;
  class StatementScope;
  ast::Statement* ConsumeStatement();
  ast::NamedNode* FindLocalMember(Token* token) const;
  bool IsBound(ast::Variable* variable) const;
  bool IsInLoop() const;
  bool IsInStatement(TokenType keyword) const;
  bool ParseBlockStatement(Token* keyword);
  bool ParseBreakStatement(Token* keyword);
  bool ParseConstStatement(Token* keyword);
  bool ParseContinueStatement(Token* keyword);
  bool ParseDoStatement(Token* keyword);
  bool ParseForStatement(Token* keyword);
  bool ParseIfStatement(Token* keyword);
  void ParseMethod(Modifiers modifiers,
                   ast::Type* method_type,
                   Token* method_name,
                   const std::vector<Token*> type_parameters);
  bool ParseReturnStatement(Token* keyword);
  bool ParseThrowStatement(Token* keyword);
  bool ParseTryStatement(Token* keyword);
  bool ParseStatement();
  bool ParseWhileStatement(Token* keyword);
  bool ParseUsingStatement(Token* keyword);
  bool ParseVarStatement(Token* keyword);
  void ParseVariables(Token* keyword, ast::Type* type);
  bool ParseYieldStatement(Token* keyword);
  ast::Statement* ProduceStatement(ast::Statement* statement);

  // Produce local variable or parameter reference at |name| token.
  ast::Expression* ProduceVariableReference(Token* name,
                                            ast::NamedNode* variable);

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
  void ParseArrayType(Token* bracket);
  bool ParseNamespaceOrTypeName();
  bool ParseType();
  std::vector<Token*> ParseTypeParameterList();
  bool ParseTypePost();
  ast::Type* ProduceType(ast::Type* type);
  ast::Type* ProduceTypeNameReference(ast::NameReference* node);
  ast::Type* ProduceTypeNameReference(Token* token);

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
