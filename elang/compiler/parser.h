// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_parser_h)
#define INCLUDE_elang_compiler_parser_h

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

namespace ast {
class Expression;
class Namespace;
class NamespaceBody;
class NamespaceMember;
class NodeFactory;
}

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
  friend class NamespaceBodyScope;

  class QualifiedNameBuilder;

  ast::NodeFactory* factory() const;

  void AddMember(ast::NamespaceMember* member);
  void Advance();
  bool AdvanceIf(TokenType type);

  // Returns last produced expression.
  ast::Expression* ConsumeExpression();

  // Returns current token and advance to next token.
  Token* ConsumeToken();

  // Returns current token as |type| and advance to next token. This function
  // is used for handling unary '+','-' and post '++', '--'.
  Token* ConsumeTokenAs(TokenType type);

  // Returns |Token*| and consume it if current token is |type|.
  Token* ConsumeTokenIf(TokenType type);

  // Returns last produced expression.
  ast::Expression* ConsumeType();

  // Always returns false for simplify error processing.
  bool Error(ErrorCode error_code);
  bool Error(ErrorCode error_code, Token* token);
  ast::NamespaceMember* FindMember(Token* token);
  Token* Parser::NewUniqueNameToken(const base::char16* format);

  // Parsing
  bool ParseAfterPrimaryExpression();
  bool ParseClassDecl();
  bool ParseEnumDecl();
  bool ParseExpression();
  bool ParseExpressionSub(ExpressionCategory category);
  bool ParseFunctionDecl();
  bool ParseCompilationUnit();
  bool ParseMethodDecl(Modifiers modifiers,
                       ast::Expression* method_type,
                       Token* method_name,
                       const std::vector<Token*> type_parameters);
  bool ParseNamespaceDecl();
  bool ParseNamespaceDecl(Token* namespace_keyword,
                          const std::vector<Token*>& names,
                          size_t index);
  bool ParseNamespaceMemberDecls();
  bool ParsePrimaryExpression();
  bool ParsePrimaryExpressionPost();
  // Returns false if no name, otherwise true.
  bool ParseQualifiedName();
  bool ParseStatement();
  bool ParseType();
  bool ParseTypePost();
  std::vector<Token*> ParseTypeParameterList();
  bool ParseUsingDirectives();
  Token* PeekToken();
  ExpressionCategory PeekTokenCategory();
  void ProduceBinaryOperation(Token* op_token, ast::Expression* left,
                              ast::Expression* right);
  void ProduceExpression(ast::Expression* expression);
  void ProduceType(ast::Expression* expression);
  void ProduceUnaryOperation(Token* op_token, ast::Expression* expression);

  void ValidateClassModifiers();
  void ValidateEnumModifiers();
  void ValidateFieldModifiers();
  void ValidateMethodModifiers();

  CompilationUnit* compilation_unit_;
  ast::Expression* expression_;
  const std::unique_ptr<Lexer> lexer_;
  int last_source_offset_;
  std::unique_ptr<ModifierParser> modifiers_;
  ast::NamespaceBody* namespace_body_;
  std::unique_ptr<QualifiedNameBuilder> name_builder_;
  CompilationSession* session_;
  Token* token_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_parser_h)
