// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/parser.h"

#include <vector>

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/lexer.h"
#include "elang/compiler/modifier.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// ModifierBuilder
//
class Parser::ModifierBuilder final {
  private: int modifiers_;
  private: std::vector<Token> tokens_;
  private: Parser* const parser_;

  public: ModifierBuilder(Parser* parser);
  public: ~ModifierBuilder() = default;

  public: const std::vector<Token>& tokens() const { return tokens_; }

  public: bool Add(const Token& token);
  public: void Reset();

  #define T(name, detaisl) \
    bool Has ## name() const { \
      return (modifiers_ & (1 << static_cast<int>(Modifier::name))) != 0; \
    }
  MODIFIER_LIST(T)
  #undef T

  DISALLOW_COPY_AND_ASSIGN(ModifierBuilder);
};

Parser::ModifierBuilder::ModifierBuilder(Parser* parser)
    : modifiers_(0), parser_(parser) {
}

bool Parser::ModifierBuilder::Add(const Token& token) {
  switch (token.type()) {
    #define CASE_CLAUSE(name, details) \
      case TokenType::name: \
        if (Has ## name()) { \
          parser_->Error(ErrorCode::SyntaxModifierDuplicate); \
          return true; \
        } \
        modifiers_ |= static_cast<int>(Modifier::name); \
        tokens_.push_back(token); \
        return true;
    MODIFIER_LIST(CASE_CLAUSE)
    #undef CASE_CLAUSE
  }
  return false;
}

void Parser::ModifierBuilder::Reset() {
  modifiers_ = 0;
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceScope
//
class Parser::NamespaceScope {
  private: Parser* const parser_;
  private: ast::Namespace* const namespace_;

  public: NamespaceScope(Parser* parser, ast::Namespace* ns);
  public: ~NamespaceScope();

  DISALLOW_COPY_AND_ASSIGN(NamespaceScope);
};

Parser::NamespaceScope::NamespaceScope(Parser* parser, ast::Namespace* ns)
    : namespace_(parser->namespace_), parser_(parser) {
  parser_->namespace_ = ns;
}

Parser::NamespaceScope::~NamespaceScope() {
  parser_->namespace_ = namespace_;
}

//////////////////////////////////////////////////////////////////////
//
// QualifiedNameBuilder
//
class Parser::QualifiedNameBuilder final {
  private: std::vector<Token> simple_names_;
  public: QualifiedNameBuilder();
  public: ~QualifiedNameBuilder() = default;

  public: const std::vector<Token> simple_names() const {
    return simple_names_;
  }

  public: void Add(const Token& simple_name);
  public: QualifiedName Get() const;
  public: bool IsSimpleName() const { return simple_names_.size() == 1u; }
  public: void Reset();

  DISALLOW_COPY_AND_ASSIGN(QualifiedNameBuilder);
};

Parser::QualifiedNameBuilder::QualifiedNameBuilder() : simple_names_(20) {
}

void Parser::QualifiedNameBuilder::Add(const Token& simple_name) {
  DCHECK(simple_name.is_name());
  simple_names_.push_back(simple_name);
}

QualifiedName Parser::QualifiedNameBuilder::Get() const {
  DCHECK_GE(simple_names_.size(), 1u);
  return QualifiedName(simple_names_);
}

void Parser::QualifiedNameBuilder::Reset() {
  simple_names_.resize(0);
}

//////////////////////////////////////////////////////////////////////
//
// Parser
//
Parser::Parser(CompilationSession* session, CompilationUnit* compilation_unit)
    : compilation_unit_(compilation_unit),
      expression_(nullptr),
      modifiers_(new ModifierBuilder(this)),
      name_builder_(new QualifiedNameBuilder()),
      namespace_(compilation_unit->global_namespace()),
      session_(session),
      lexer_(new Lexer(session, compilation_unit)) {
}

Parser::~Parser() {
}

ast::NodeFactory* Parser::factory() const {
  return session_->ast_factory();
}

void Parser::Advance() {
 token_ = Token();
}

bool Parser::AdvanceIf(TokenType type) {
  if (PeekToken() != type)
    return false;
  Advance();
  return true;
}

bool Parser::Error(ErrorCode error_code, const Token& token) {
  DCHECK_NE(token.type(), TokenType::None);
  session_->AddError(token_.location(), error_code,
                     std::vector<Token> { token });
  return false;
}

bool Parser::Error(ErrorCode error_code) {
  return Error(error_code, token_);
}

// ClassDecl ::= Attribute* ClassModifier* "partial"? "class"
//               Name TypeParamereList? ClassBase?
//               TypeParameterConstraintsClasses?
//               ClassBody ";"?
// ClassModifier ::= ClassModifierAccessibility |
//                   ClassModifierKind |
//                   "new"
// ClassModifierAccessibility := "private" | "protected" | "public"
// ClassModifierKind := "abstract" | "static" | "final"
//
// ClassBody ::= "{" ClassMemberDecl* "}"
// 

bool Parser::ParseClassDecl() {
  // Check class modifiers
  {
    auto has_accessibility = false;
    auto has_inheritance = false;
    for (const auto& token : modifiers_->tokens()) {
      switch (token.type()) {
        case TokenType::Abstract:
        case TokenType::New:
        case TokenType::Static:
          if (has_inheritance)
            Error(ErrorCode::SyntaxClassDeclModifier, token);
          else
              has_inheritance = true;
          break;
        case TokenType::Private:
        case TokenType::Protected:
        case TokenType::Public:
          if (has_accessibility)
            Error(ErrorCode::SyntaxClassDeclModifier, token);
          else
            has_accessibility = true;
          break;
        case TokenType::Virtual:
        case TokenType::Volatile:
          Error(ErrorCode::SyntaxClassDeclModifier, token);
          break;
      }
    }
  }
  auto class_keyword = token_;
  Advance();
  PeekToken();
  auto simple_name = token_;
  if (!simple_name.is_name())
    return Error(ErrorCode::SyntaxClassDeclName);

  auto const klass = factory()->NewClass(namespace_, class_keyword,
                                         simple_name);
  NamespaceScope member_scope(this, klass);

  // TypeParameterList
  if (AdvanceIf(TokenType::LeftAngleBracket)) {
    for (;;) {
      if (!ParseTypeParameter())
        return false;
      if (AdvanceIf(TokenType::Lt))
        break;
      if (!AdvanceIf(TokenType::Comma))
        return Error(ErrorCode::SyntaxClassDeclTypeParamInvalid);
    }
  }

  // ClassBase
  if (AdvanceIf(TokenType::Colon)) {
    while (ParseQualifiedName()) {
      klass->AddBaseClassName(name_builder_->Get());
      if (AdvanceIf(TokenType::Comma))
        break;
    }
  }

  // ClassBody ::= "{" ClassMemberDeclaration* "}"
  // ClassMemberDeclaration ::=
  //    ConstantDecl |
  //    FieldDecl |
  //    MethodDecl |
  //    PropertyDecl |
  //    IndexerDecl |
  //    OperatorDecl |
  //    ConstructorDecl |
  //    FinalizerDecl |
  //    StaticConstructorDecl |
  //    TypeDecl
  if (!AdvanceIf(TokenType::LeftCurryBracket))
    return Error(ErrorCode::SyntaxClassDeclLeftCurryBracket);

  for (;;) {
    modifiers_->Reset();
    PeekToken();
    while (modifiers_->Add(token_)) {
      Advance();
      PeekToken();
    }

    switch (PeekToken()) {
      case TokenType::Class:
        ParseClassDecl();
        continue;
      case TokenType::Enum:
        ParseEnumDecl();
        continue;
      case TokenType::Function:
        ParseFunctionDecl();
        continue;
    }

    // FieldDecl ::= Type Name ("=" Expression)? ";"
    // MethodDecl ::= Type Name ParameterDecl ";"
    //                Type Name ParameterDecl "{" Statement* "}"
    if (!ParseMaybeType())
      break;
    PeekToken();
    if (!token_.is_name())
      return Error(ErrorCode::SyntaxFieldDeclName);
    if (!AdvanceIf(TokenType::SemiColon))
      Error(ErrorCode::SyntaxFieldDeclSemiColon);
    break;
  }

  if (!AdvanceIf(TokenType::RightCurryBracket))
    return Error(ErrorCode::SyntaxClassDeclRightCurryBracket);
  return true;
}

// CompilationUnit ::=
//      ExternalAliasDirective
//      UsingDirective*
//      GlobalAttribute*
//      NamespaceMemberDecl*
bool Parser::ParseCompilationUnit() {
  if (!ParseUsingDirectives() || !ParseNamespaceMemberDecls())
    return false;
  return PeekToken() == TokenType::EndOfSource;
}

// EnumDecl := EnumModifier* "enum" Name "{" EnumField* "}"
// EnumField ::= Name ("=" Expression)? ","?
bool Parser::ParseEnumDecl() {
  DCHECK_EQ(token_.type(), TokenType::Enum);
  auto enum_keyword = token_;
  Advance();
  PeekToken();
  if (!token_.is_name())
    return Error(ErrorCode::SyntaxEnumDeclNameInvalid);
  auto enum_name = token_;
  if (namespace_->FindMember(enum_name))
    Error(ErrorCode::SyntaxEnumDeclNameDuplicate);
  auto const enum_decl = factory()->NewEnum(namespace_, enum_keyword,
                                            enum_name);
  namespace_->AddMember(enum_decl);
  Advance();
  if (!AdvanceIf(TokenType::LeftCurryBracket))
    return Error(ErrorCode::SyntaxEnumDeclLeftCurryBracket);
  for (;;) {
    PeekToken();
    if (!token_.is_name())
      break;
    auto member_name = token_;
    Advance();
    DCHECK(!expression_);
    if (AdvanceIf(TokenType::Assign))
      ParseExpression();
    enum_decl->AddMember(factory()->NewEnumMember(enum_decl, enum_name,
                                                  expression_));
    if (PeekToken() == TokenType::RightCurryBracket)
      break;
    if (AdvanceIf(TokenType::Comma))
      continue;
  }
  if (!AdvanceIf(TokenType::RightCurryBracket))
    return Error(ErrorCode::SyntaxEnumDeclRightCurryBracket);
  return true;
}

void Parser::ParseExpression() {
  expression_ = nullptr;
}

bool Parser::ParseFunctionDecl() {
  return false;
}

bool Parser::ParseMaybeType() {
  return false;
}

//  NamespaceDecl ::= "namespace" QualifiedName Namespace ";"?
//  Namespace ::= "{" ExternAliasDirective* UsingDirective*
//                        NamespaceMemberDecl* "}"
bool Parser::ParseNamespaceDecl() {
  DCHECK_EQ(token_.type(), TokenType::Namespace);
  auto namespace_keyword = token_;
  Advance();
  if (!ParseQualifiedName())
    return false;
  auto const ns = factory()->NewNamespace(namespace_, namespace_keyword,
                                          name_builder_->Get());
  namespace_->AddMember(ns);
  NamespaceScope namespace_scope(this, ns);
  // TODO(eval1749) Record position of left bracket for error message
  // when there is no matching right bracket.
  if (!AdvanceIf(TokenType::LeftCurryBracket))
    return Error(ErrorCode::SyntaxNamespaceDeclLeftCurryBracket);
  if (!ParseUsingDirectives())
    return false;
  if (!ParseNamespaceMemberDecls())
    return false;
  // TODO(eval1749) Report unmatched left bracket when token is EOS.
  if (!AdvanceIf(TokenType::RightCurryBracket))
    return Error(ErrorCode::SyntaxNamespaceDeclRightCurryBracket);
  AdvanceIf(TokenType::SemiColon);
  return true;
}

// NamespaceMemberDecl ::= NamespaceDecl | TypeDecl
// TypeDecl ::= ClassDecl | InterfaceDecl | StructDecl | EnumDecl |
//              FunctionDecl
bool Parser::ParseNamespaceMemberDecls() {
  for (;;) {
    modifiers_->Reset();
    PeekToken();
    while (modifiers_->Add(token_)) {
      Advance();
      PeekToken();
    }
    switch (PeekToken()) {
      case TokenType::Class:
      case TokenType::Interface:
      case TokenType::Struct:
        if (!ParseClassDecl())
          return false;
        break;
      case TokenType::Enum:
        if (!ParseEnumDecl())
          return false;
        break;
      case TokenType::Function:
        if (!ParseFunctionDecl())
          return false;
        break;
      case TokenType::Namespace:
        if (!ParseNamespaceDecl())
          return false;
        break;
      default:
        // TODO(eval1749) Report unmatched right bracket if there is no
        // matching bracket.
        return true;
    }
  }
}

bool Parser::ParseQualifiedName() {
  name_builder_->Reset();
  for (;;) {
    PeekToken();
    if (!token_.is_name())
      break;
    name_builder_->Add(token_);
    Advance();
    if (!AdvanceIf(TokenType::Dot))
      return true;
  }
  return false;
}

bool Parser::ParseTypeParameter() {
  return false;
}

// UsingDirective ::= AliasDef | ImportNamespace
// AliasDef ::= "using" Name "="  QualfiedName ";"
// ImportNamespace ::= "using" QualfiedName ";"
bool Parser::ParseUsingDirectives() {
  while (PeekToken() == TokenType::Using) {
    auto using_keyword = token_;
    Advance();
    if (!ParseQualifiedName())
      return Error(ErrorCode::SyntaxUsingDirectiveName);
    if (AdvanceIf(TokenType::Eq)) {
      if (!name_builder_->IsSimpleName())
        return Error(ErrorCode::SyntaxAliasDefAliasName);
      auto alias_name = name_builder_->simple_names().front();
      if (!ParseQualifiedName())
        return Error(ErrorCode::SyntaxAliasDefRealName);
      namespace_->AddAlias(alias_name, std::move(name_builder_->Get()));
    } else {
      namespace_->AddImport(using_keyword, std::move(name_builder_->Get()));
    }
    if (!AdvanceIf(TokenType::SemiColon))
      return Error(ErrorCode::SyntaxUsingDirectiveSemiColon);
  }
  return true;
}

TokenType Parser::PeekToken() {
  if (token_.type() != TokenType::None)
    return token_.type();
  token_ = lexer_->GetToken();
  return token_.type();
}

bool Parser::Run() {
  return ParseCompilationUnit();
}

}  // namespace compiler
}  // namespace elang
