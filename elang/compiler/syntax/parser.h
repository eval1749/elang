// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SYNTAX_PARSER_H_
#define ELANG_COMPILER_SYNTAX_PARSER_H_

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

namespace ast {
class NamedNode;
class Expression;
class LocalVariable;
class Namespace;
class NamespaceBody;
class NamespaceMember;
class NodeFactory;
class Statement;
class VarStatement;
}  // namespace ast

class CompilationUnit;
class CompilationSession;
enum class ErrorCode;
class Lexer;
class Modifiers;
class QualifiedName;
class SourceCodePosition;

//////////////////////////////////////////////////////////////////////
//
// Parser
//
class Parser final {
  // |ExpressionCategory| representing precedence of operator.
  // Note: To implement operation of |ExpressionCategory| in "parser.cc"
  // it is marked |public|, but other modules can't use it.
 public:
  enum class ExpressionCategory;

  Parser(CompilationSession* session, CompilationUnit* compilation_unit);
  ~Parser();

  CompilationSession* session() const { return session_; }

  // Parser entry point. Returns true if parsing succeeded, otherwise false.
  bool Run();

 private:
  class ModifierParser;
  class NamespaceBodyScope;

  class QualifiedNameBuilder;

  ast::NodeFactory* factory() const;

  void AddMember(ast::NamespaceMember* member);
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
  ast::NamespaceMember* FindMember(Token* token) const;
  Token* Parser::NewUniqueNameToken(const base::char16* format);

  bool ParseClassDecl();
  bool ParseEnumDecl();
  bool ParseFunctionDecl();
  bool ParseCompilationUnit();
  bool ParseNamespaceDecl();
  bool ParseNamespaceDecl(Token* namespace_keyword,
                          const std::vector<Token*>& names,
                          size_t index);
  bool ParseNamespaceMemberDecls();
  bool ParseQualifiedName();
  bool ParseUsingDirectives();
  Token* PeekToken();
  ast::NamespaceMember* ResolveMember(Token* token) const;
  void ValidateClassModifiers();
  void ValidateEnumModifiers();
  void ValidateFieldModifiers();
  void ValidateMethodModifiers();

  // in "parse_expression.cc"
  // Returns last produced expression.
  ast::Expression* ConsumeExpression();
  bool ParseAfterPrimaryExpression();
  bool ParseExpression();
  bool ParseExpressionSub(ExpressionCategory category);
  bool ParsePrimaryExpression();
  bool ParsePrimaryExpressionPost();
  bool ParseUnaryExpression();
  ExpressionCategory PeekTokenCategory();
  ast::Expression* ProduceBinaryOperation(Token* op_token,
                                          ast::Expression* left,
                                          ast::Expression* right);
  ast::Expression* ProduceExpression(ast::Expression* expression);
  ast::Expression* ProduceUnaryOperation(Token* op_token,
                                         ast::Expression* expression);
  Token* TryConsumeUnaryOperator();

  // in "parse_statement.cc"
  // Returns last produced statement.
  class LocalDeclarationSpace;
  class StatementScope;
  ast::Statement* ConsumeStatement();
  ast::NamedNode* FindLocalMember(Token* token) const;
  bool IsInLoop() const;
  bool IsInStatement(TokenType keyword) const;
  bool ParseBlockStatement(Token* keyword);
  bool ParseBreakStatement(Token* keyword);
  bool ParseConstStatement(Token* keyword);
  bool ParseContinueStatement(Token* keyword);
  bool ParseDoStatement(Token* keyword);
  bool ParseForStatement(Token* keyword);
  bool ParseIfStatement(Token* keyword);
  bool ParseMethodDecl(Modifiers modifiers,
                       ast::Expression* method_type,
                       Token* method_name,
                       const std::vector<Token*> type_parameters);
  bool ParseReturnStatement(Token* keyword);
  bool ParseThrowStatement(Token* keyword);
  bool ParseTryStatement(Token* keyword);
  bool ParseStatement();
  bool ParseWhileStatement(Token* keyword);
  bool ParseUsingStatement(Token* keyword);
  bool ParseVarStatement(Token* keyword);
  void ParseVariables(Token* keyword, ast::Expression* type);
  bool ParseYieldStatement(Token* keyword);
  ast::Statement* ProduceStatement(ast::Statement* statement);

  // in "parse_type.cc"
  // Returns last produced expression.
  ast::Expression* ConsumeType();
  // Returns true if |expression| can be type. Since we've not yet resolved
  // name references, |expression| may not be type.
  bool MaybeType(ast::Expression* expression) const;
  bool ParseNamespaceOrTypeName();
  bool ParseType();
  std::vector<Token*> ParseTypeParameterList();
  bool ParseTypePost();
  ast::Expression* ProduceMemberAccess(
      const std::vector<ast::Expression*>& names);
  ast::Expression* ProduceType(ast::Expression* expression);

  CompilationUnit* compilation_unit_;
  LocalDeclarationSpace* declaration_space_;
  ast::Expression* expression_;
  const std::unique_ptr<Lexer> lexer_;
  int last_source_offset_;
  std::unique_ptr<ModifierParser> modifiers_;
  ast::NamespaceBody* namespace_body_;
  std::unique_ptr<QualifiedNameBuilder> name_builder_;
  CompilationSession* session_;
  ast::Statement* statement_;
  StatementScope* statement_scope_;
  Token* token_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SYNTAX_PARSER_H_
