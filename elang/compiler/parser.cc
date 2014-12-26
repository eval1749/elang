// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/parser.h"

#include <vector>

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
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
  private: std::vector<Token*> tokens_;
  private: Parser* const parser_;

  public: ModifierBuilder(Parser* parser);
  public: ~ModifierBuilder() = default;

  public: const std::vector<Token*>& tokens() const { return tokens_; }

  public: bool Add(Token* token);
  public: void Reset();

  #define T(name, details) \
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

bool Parser::ModifierBuilder::Add(Token* token) {
  switch (token->type()) {
    #define CASE_CLAUSE(name, details) \
      case TokenType::name: \
        if (Has ## name()) { \
          parser_->Error(ErrorCode::SyntaxModifierDuplicate); \
          return true; \
        } \
        modifiers_ |= 1 << static_cast<int>(Modifier::name); \
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
// NamespaceBodyScope
//
class Parser::NamespaceBodyScope {
  private: Parser* const parser_;
  private: ast::NamespaceBody* const namespace_body_;

  public: NamespaceBodyScope(Parser* parser, ast::Namespace* new_namespace);
  public: ~NamespaceBodyScope();

  DISALLOW_COPY_AND_ASSIGN(NamespaceBodyScope);
};

Parser::NamespaceBodyScope::NamespaceBodyScope(Parser* parser,
                                               ast::Namespace* new_namespace)
    : namespace_body_(parser->namespace_body_), parser_(parser) {
  auto const namespace_body = new ast::NamespaceBody(namespace_body_,
                                                     new_namespace);
  new_namespace->AddNamespaceBody(namespace_body);
  parser->namespace_body_ = namespace_body;
}

Parser::NamespaceBodyScope::~NamespaceBodyScope() {
  parser_->namespace_body_ = namespace_body_;
}

//////////////////////////////////////////////////////////////////////
//
// QualifiedNameBuilder
//
class Parser::QualifiedNameBuilder final {
  private: std::vector<Token*> simple_names_;
  public: QualifiedNameBuilder();
  public: ~QualifiedNameBuilder() = default;

  public: const std::vector<Token*> simple_names() const {
    return simple_names_;
  }

  public: void Add(Token* simple_name);
  public: QualifiedName Get() const;
  public: bool IsSimpleName() const { return simple_names_.size() == 1u; }
  public: void Reset();

  DISALLOW_COPY_AND_ASSIGN(QualifiedNameBuilder);
};

Parser::QualifiedNameBuilder::QualifiedNameBuilder() : simple_names_(20) {
}

void Parser::QualifiedNameBuilder::Add(Token* simple_name) {
  DCHECK(simple_name->is_name());
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
      lexer_(new Lexer(session, compilation_unit)),
      modifiers_(new ModifierBuilder(this)),
      name_builder_(new QualifiedNameBuilder()),
      namespace_body_(new ast::NamespaceBody(nullptr,
                                             session->global_namespace())),
      session_(session),
      token_(nullptr) {
  namespace_body_->owner()->AddNamespaceBody(namespace_body_);
}

Parser::~Parser() {
}

ast::NodeFactory* Parser::factory() const {
  return session_->ast_factory();
}

void Parser::AddMember(ast::NamespaceMember* member) {
  DCHECK(!member->is<ast::Alias>());
  namespace_body_->AddMember(member);
}

void Parser::Advance() {
 DCHECK(token_);
 token_ = nullptr;
}

bool Parser::AdvanceIf(TokenType type) {
  if (PeekToken() != type)
    return false;
  Advance();
  return true;
}

bool Parser::Error(ErrorCode error_code, Token* token) {
  DCHECK(token);
  session_->AddError(error_code, token);
  return false;
}

bool Parser::Error(ErrorCode error_code) {
  return Error(error_code, token_);
}

ast::NamespaceMember* Parser::FindMember(Token* simple_name) {
  return namespace_body_->FindMember(simple_name);
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
      switch (token->type()) {
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
  if (!simple_name->is_name())
    return Error(ErrorCode::SyntaxClassDeclName);
  if (FindMember(simple_name))
    Error(ErrorCode::SyntaxClassDeclNameDuplicate);
  Advance();
  auto const clazz = factory()->NewClass(namespace_body_, class_keyword,
                                         simple_name);
  AddMember(clazz);
  NamespaceBodyScope namespace_body_scope(this, clazz);

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
      clazz->AddBaseClassName(name_builder_->Get());
      if (!AdvanceIf(TokenType::Comma))
        break;
    }
  }

  if (modifiers_->HasExtern()) {
    if (!AdvanceIf(TokenType::SemiColon))
      Error(ErrorCode::SyntaxClassDeclSemiColon);
    return true;
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
    if (!token_->is_name())
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
  DCHECK_EQ(token_->type(), TokenType::Enum);
  auto enum_keyword = token_;
  Advance();
  PeekToken();
  if (!token_->is_name())
    return Error(ErrorCode::SyntaxEnumDeclNameInvalid);
  auto enum_name = token_;
  if (FindMember(enum_name))
    Error(ErrorCode::SyntaxEnumDeclNameDuplicate);
  auto const enum_decl = factory()->NewEnum(namespace_body_, enum_keyword,
                                            enum_name);
  AddMember(enum_decl);
  Advance();
  if (!AdvanceIf(TokenType::LeftCurryBracket))
    return Error(ErrorCode::SyntaxEnumDeclLeftCurryBracket);
  for (;;) {
    PeekToken();
    if (!token_->is_name())
      break;
    auto const member_name = token_;
    Advance();
    DCHECK(!expression_);
    if (AdvanceIf(TokenType::Assign))
      ParseExpression();
    enum_decl->AddMember(factory()->NewEnumMember(enum_decl, member_name,
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
  DCHECK_EQ(token_->type(), TokenType::Namespace);
  auto namespace_keyword = token_;
  Advance();
  if (!ParseQualifiedName())
    return false;
  const auto name = name_builder_->Get();
  return ParseNamespaceDecl(namespace_keyword, name.simple_names(), 0);
}

bool Parser::ParseNamespaceDecl(Token* namespace_keyword,
                                const std::vector<Token*>& names,
                                size_t index) {
  auto const simple_name = names[index];
  ast::Namespace* new_namespace = nullptr;
  if (auto const present = FindMember(simple_name)) {
    new_namespace = present->ToNamespace();
    if (!new_namespace)
      Error(ErrorCode::SyntaxNamespaceDeclNameDuplicate, simple_name);
  }
  if (!new_namespace) {
    new_namespace = factory()->NewNamespace(namespace_body_,
                                            namespace_keyword, simple_name);
    AddMember(new_namespace);
  }
  NamespaceBodyScope namespace_body_scope(this, new_namespace);
  if (index + 1 < names.size())
    return ParseNamespaceDecl(namespace_keyword, names, index + 1);
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
    if (!token_->is_name())
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
  DCHECK(namespace_body_->owner()->ToNamespace());
  while (PeekToken() == TokenType::Using) {
    auto using_keyword = token_;
    Advance();
    if (!ParseQualifiedName())
      return Error(ErrorCode::SyntaxUsingDirectiveName);
    if (AdvanceIf(TokenType::Assign)) {
      if (!name_builder_->IsSimpleName())
        return Error(ErrorCode::SyntaxAliasDefAliasName);
      auto alias_name = name_builder_->simple_names().front();
      if (!ParseQualifiedName())
        return Error(ErrorCode::SyntaxAliasDefRealName);
      namespace_body_->AddAlias(factory()->NewAlias(
          namespace_body_, using_keyword, alias_name, name_builder_->Get()));
    } else {
      namespace_body_->AddImport(using_keyword, name_builder_->Get());
    }
    if (!AdvanceIf(TokenType::SemiColon))
      return Error(ErrorCode::SyntaxUsingDirectiveSemiColon);
  }
  return true;
}

TokenType Parser::PeekToken() {
  if (token_)
    return token_->type();
  token_ = lexer_->GetToken();
  return token_->type();
}

bool Parser::Run() {
  return ParseCompilationUnit();
}

}  // namespace compiler
}  // namespace elang
