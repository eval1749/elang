// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_parser_h)
#define INCLUDE_elang_compiler_parser_h

#include <memory>

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
class QualifiedName;

//////////////////////////////////////////////////////////////////////
//
// Parser
//
class Parser final {
  // |ExpressionCategory| representing precedence of operator.
  // Note: To implement operation of |ExpressionCategory| in "parser.cc"
  // it is marked |public|, but other modules can't use it.
  public: enum class ExpressionCategory;
  private: class ModifierBuilder;
  private: class NamespaceBodyScope;
  friend class NamespaceBodyScope;
  private: class QualifiedNameBuilder;

  private: CompilationUnit* compilation_unit_;
  private: ast::Expression* expression_;
  private: const std::unique_ptr<Lexer> lexer_;
  private: std::unique_ptr<ModifierBuilder> modifiers_;
  private: ast::NamespaceBody* namespace_body_;
  private: std::unique_ptr<QualifiedNameBuilder> name_builder_;
  private: CompilationSession* session_;
  private: Token* token_;

  public: Parser(CompilationSession* session,
                 CompilationUnit* compilation_unit);
  public: ~Parser();

  private: ast::NodeFactory* factory() const;
  public: CompilationSession* session() const { return session_; }

  private: void AddMember(ast::NamespaceMember* member);
  private: void Advance();
  private: bool AdvanceIf(TokenType type);

  // Returns last produced expression.
  private: ast::Expression* ConsumeExpression();

  // Returns current token and advance to next token.
  private: Token* ConsumeToken();

  // Returns current token as |type| and advance to next token. This function
  // is used for handling unary '+','-' and post '++', '--'.
  private: Token* ConsumeTokenAs(TokenType type);

  // Returns |Token*| and consume it if current token is |type|.
  private: Token* ConsumeTokenIf(TokenType type);

  // Always returns false for simplify error processing.
  private: bool Error(ErrorCode error_code);
  private: bool Error(ErrorCode error_code, Token* token);
  private: ast::NamespaceMember* FindMember(Token* token);
  private: bool ParseAfterPrimaryExpression();
  private: bool ParseClassDecl();
  private: bool ParseEnumDecl();
  private: bool ParseExpression();
  private: bool ParseExpressionSub(ExpressionCategory category);
  private: bool ParseFunctionDecl();
  private: bool ParseCompilationUnit();
  private: bool ParseMaybeType();
  private: bool ParseNamespaceDecl();
  private: bool ParseNamespaceDecl(Token* namespace_keyword,
                                   const std::vector<Token*>& names,
                                   size_t index);
  private: bool ParseNamespaceMemberDecls();
  private: bool ParsePrimaryExpression();
  private: bool ParsePrimaryExpressionPost();
  // Returns false if no name, otherwise true.
  private: bool ParseQualifiedName();
  private: bool ParseTypeParameter();
  private: bool ParseUsingDirectives();
  private: TokenType PeekToken();
  private: ExpressionCategory PeekTokenCategory();
  private: void ProduceBinaryOperation(Token* op_token,
                                       ast::Expression* left,
                                       ast::Expression* right);
  private: void ProduceExpression(ast::Expression* expression);
  private: void ProduceUnaryOperation(Token* op_token,
                                      ast::Expression* expression);

  // Parser entry point. Returns true if parsing succeeded, otherwise false.
  public: bool Run();

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_parser_h)

