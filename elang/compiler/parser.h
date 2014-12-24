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
class Namespace;
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
  private: class ModifierBuilder;
  private: class NamespaceScope;
  friend class NamespaceScope;
  private: class QualifiedNameBuilder;

  private: CompilationUnit* compilation_unit_;
  private: ast::Expression* expression_;
  private: ast::Namespace* namespace_;
  private: std::unique_ptr<ModifierBuilder> modifiers_;
  private: std::unique_ptr<QualifiedNameBuilder> name_builder_;
  private: CompilationSession* session_;
  private: Token token_;
  private: const std::unique_ptr<Lexer> lexer_;

  public: Parser(CompilationSession* session,
                 CompilationUnit* compilation_unit);
  public: ~Parser();

  private: ast::NodeFactory* factory() const;
  public: CompilationSession* session() const { return session_; }

  private: void Advance();
  private: bool AdvanceIf(TokenType type);
  // Always returns false for simplify error processing.
  private: bool Error(ErrorCode error_code);
  private: bool Error(ErrorCode error_code, const Token& token);
  private: bool ParseClassDecl();
  private: bool ParseEnumDecl();
  private: void ParseExpression();
  private: bool ParseFunctionDecl();
  private: bool ParseCompilationUnit();
  private: bool ParseMaybeType();
  private: bool ParseNamespaceDecl();
  private: bool ParseNamespaceMemberDecls();
  // Returns false if no name, otherwise true.
  private: bool ParseQualifiedName();
  private: bool ParseTypeParameter();
  private: bool ParseUsingDirectives();
  private: TokenType PeekToken();
  public: bool Run();

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_parser_h)

