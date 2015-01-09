// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/syntax/parser.h"

#include <string>
#include <vector>
#include <unordered_set>

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/import.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/syntax/lexer.h"
#include "elang/compiler/modifiers_builder.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {
Token* GetQualifiedNameToken(ast::Expression* thing) {
  if (auto const name_reference = thing->as<ast::NameReference>())
    return name_reference->name();
  auto const member_access = thing->as<ast::MemberAccess>();
  if (!member_access)
    return false;
  for (auto const component : member_access->components()) {
    if (!component->is<ast::NameReference>())
      return nullptr;
  }
  return member_access->token();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Parser::ModifierParser
//
class Parser::ModifierParser final {
 public:
  explicit ModifierParser(Parser* parser);
  ~ModifierParser() = default;

  const std::vector<Token*>& tokens() const { return tokens_; }

  bool Add(Token* token);
  Modifiers Get() const;
  void Reset();

 private:
  ModifiersBuilder builder_;
  std::vector<Token*> tokens_;
  Parser* const parser_;

  DISALLOW_COPY_AND_ASSIGN(ModifierParser);
};

Parser::ModifierParser::ModifierParser(Parser* parser) : parser_(parser) {
}

bool Parser::ModifierParser::Add(Token* token) {
  if (builder_.HasPartial())
    parser_->Error(ErrorCode::SyntaxModifierPartial);
  switch (token->type()) {
#define CASE_CLAUSE(name, string, details)                \
  case TokenType::name:                                   \
    if (builder_.Has##name()) {                           \
      parser_->Error(ErrorCode::SyntaxModifierDuplicate); \
      return true;                                        \
    }                                                     \
    builder_.Set##name();                                 \
    tokens_.push_back(token);                             \
    return true;
    FOR_EACH_MODIFIER(CASE_CLAUSE)
#undef CASE_CLAUSE
  }
  return false;
}

Modifiers Parser::ModifierParser::Get() const {
  return builder_.Get();
}

void Parser::ModifierParser::Reset() {
  builder_.Reset();
}

//////////////////////////////////////////////////////////////////////
//
// NamespaceBodyScope
//
class Parser::NamespaceBodyScope {
 public:
  NamespaceBodyScope(Parser* parser, ast::MemberContainer* new_container);
  ~NamespaceBodyScope();

 private:
  Parser* const parser_;
  ast::NamespaceBody* const namespace_body_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBodyScope);
};

Parser::NamespaceBodyScope::NamespaceBodyScope(
    Parser* parser,
    ast::MemberContainer* new_container)
    : namespace_body_(parser->namespace_body_), parser_(parser) {
  auto const namespace_body = parser->session_->ast_factory()->NewNamespaceBody(
      namespace_body_, new_container);
  new_container->AddNamespaceBody(namespace_body);
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
 public:
  QualifiedNameBuilder();
  ~QualifiedNameBuilder() = default;

  const std::vector<Token*> simple_names() const { return simple_names_; }

  void Add(Token* simple_name);
  QualifiedName Get() const;
  bool IsAtomicString() const { return simple_names_.size() == 1u; }
  void Reset();

 private:
  std::vector<Token*> simple_names_;

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
      declaration_space_(nullptr),
      expression_(nullptr),
      last_source_offset_(0),
      lexer_(new Lexer(session, compilation_unit)),
      modifiers_(new ModifierParser(this)),
      name_builder_(new QualifiedNameBuilder()),
      namespace_body_(session->ast_factory()->NewNamespaceBody(
          nullptr,
          session->global_namespace())),
      session_(session),
      statement_(nullptr),
      statement_scope_(nullptr),
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
  PeekToken();
}

bool Parser::AdvanceIf(TokenType type) {
  if (PeekToken() != type)
    return false;
  Advance();
  return true;
}

Token* Parser::ConsumeToken() {
  DCHECK(token_);
  auto const result = token_;
  Advance();
  return result;
}

Token* Parser::ConsumeTokenIf(TokenType type) {
  if (PeekToken() != type)
    return nullptr;
  return ConsumeToken();
}

bool Parser::Error(ErrorCode error_code, Token* token, Token* token2) {
  DCHECK(token);
  session_->AddError(error_code, token, token2);
  return false;
}

bool Parser::Error(ErrorCode error_code, Token* token) {
  DCHECK(token);
  session_->AddError(error_code, token);
  return false;
}

bool Parser::Error(ErrorCode error_code) {
  return Error(error_code, token_);
}

ast::NamespaceMember* Parser::FindMember(Token* name) const {
  if (auto const present = namespace_body_->FindAlias(name))
    return present;
  if (auto const present = namespace_body_->FindMember(name))
    return present;
  return nullptr;
}

Token* Parser::NewUniqueNameToken(const base::char16* format) {
  return session_->NewUniqueNameToken(
      SourceCodeRange(compilation_unit_->source_code(), last_source_offset_,
                      last_source_offset_),
      format);
}

// ClassDecl ::= Attribute* ClassModifier* 'partial'? 'class'
//               Name TypeParamereList? ClassBase?
//               TypeParameterConstraintsClasses?
//               ClassBody ';'?
// ClassModifier ::= ClassModifierAccessibility |
//                   ClassModifierKind |
//                   'new'
// ClassModifierAccessibility := 'private' | 'protected' | 'public'
// ClassModifierKind := 'abstract' | 'static' | 'final'
//
// ClassBody ::= '{' ClassMemberDecl* '}'
//
bool Parser::ParseClassDecl() {
  ValidateClassModifiers();
  // TODO(eval1749) Support partial class.
  auto const class_modifiers = modifiers_->Get();
  auto const class_keyword = ConsumeToken();
  auto const class_name = ConsumeToken();
  if (!class_name->is_name())
    return Error(ErrorCode::SyntaxClassDeclName, class_name);
  if (FindMember(class_name))
    Error(ErrorCode::SyntaxClassDeclNameDuplicate, class_name);
  auto const clazz = factory()->NewClass(namespace_body_, class_modifiers,
                                         class_keyword, class_name);
  AddMember(clazz);
  NamespaceBodyScope namespace_body_scope(this, clazz);

  // TypeParameterList
  if (AdvanceIf(TokenType::LeftAngleBracket))
    ParseTypeParameterList();

  // ClassBase
  if (AdvanceIf(TokenType::Colon)) {
    while (ParseNamespaceOrTypeName()) {
      clazz->AddBaseClassName(ConsumeType());
      if (!AdvanceIf(TokenType::Comma))
        break;
    }
  }

  if (class_modifiers.HasExtern()) {
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
    while (modifiers_->Add(PeekToken()))
      Advance();

    switch (PeekToken()->type()) {
      case TokenType::Class:
      case TokenType::Interface:
      case TokenType::Struct:
        ParseClassDecl();
        continue;
      case TokenType::Enum:
        ParseEnumDecl();
        continue;
      case TokenType::Function:
        ParseFunctionDecl();
        continue;
      case TokenType::RightCurryBracket:
        Advance();
        return true;
    }

    // MethodDecl ::=
    //    Type Name TypeParameterList? ParameterDecl ';'
    //    Type Name TypeParameterList? ParameterDecl '{'
    //    Statement* '}'
    if (auto const var_keyword = ConsumeTokenIf(TokenType::Var)) {
      ProduceType(factory()->NewNameReference(var_keyword));
    } else if (!ParseType()) {
      return Error(ErrorCode::SyntaxClassDeclRightCurryBracket);
    }
    // TODO(eval1749) Validate FieldMoifiers
    auto const member_modifiers = modifiers_->Get();
    auto const member_type = ConsumeType();
    auto const member_name = ConsumeToken();
    if (!member_name->is_name())
      return Error(ErrorCode::SyntaxClassMemberName);
    if (AdvanceIf(TokenType::LeftAngleBracket)) {
      auto const type_parameters = ParseTypeParameterList();
      if (!AdvanceIf(TokenType::LeftParenthesis)) {
        Error(ErrorCode::SyntaxClassMemberParenthesis);
        // TODO(eval1749) Skip until '{' or '}'
        continue;
      }
      ParseMethodDecl(member_modifiers, member_type, member_name,
                      type_parameters);
      continue;
    }
    if (AdvanceIf(TokenType::LeftParenthesis)) {
      ParseMethodDecl(member_modifiers, member_type, member_name, {});
      continue;
    }

    // FieldDecl ::= Type Name ('=' Expression)? ';'
    //
    if (FindMember(member_name))
      Error(ErrorCode::SyntaxClassMemberDuplicate, member_name);
    ValidateFieldModifiers();
    if (AdvanceIf(TokenType::Assign)) {
      if (!ParseExpression())
        return false;
      AddMember(factory()->NewField(namespace_body_, member_modifiers,
                                    member_type, member_name,
                                    ConsumeExpression()));
      if (!AdvanceIf(TokenType::SemiColon))
        Error(ErrorCode::SyntaxClassMemberSemiColon);
      continue;
    }

    // |var| field must have initial value.
    if (auto const name_ref = member_type->as<ast::NameReference>()) {
      if (name_ref->name() == TokenType::Var)
        Error(ErrorCode::SyntaxClassMemberVarField, member_name);
    }

    AddMember(factory()->NewField(namespace_body_, member_modifiers,
                                  member_type, member_name, nullptr));
    if (!AdvanceIf(TokenType::SemiColon))
      Error(ErrorCode::SyntaxClassMemberSemiColon);
  }
}

// CompilationUnit ::=
//      ExternalAliasDirective
//      UsingDirective*
//      GlobalAttribute*
//      NamespaceMemberDecl*
bool Parser::ParseCompilationUnit() {
  if (!ParseUsingDirectives() || !ParseNamespaceMemberDecls())
    return false;
  if (PeekToken() == TokenType::EndOfSource)
    return true;
  Error(ErrorCode::SyntaxCompilationUnitInvalid, token_);
  return false;
}

// EnumDecl := EnumModifier* "enum" Name EnumBase? "{" EnumField* "}"
// EnumBase ::= ':' IntegralType
// EnumField ::= Name ("=" Expression)? ","?
// EnumModifier ::= 'new' | 'public' | 'protected' | 'private'
bool Parser::ParseEnumDecl() {
  // TODO(eval1749) Validate EnumModifier
  auto const enum_modifiers = modifiers_->Get();
  auto const enum_keyword = ConsumeToken();
  DCHECK_EQ(enum_keyword->type(), TokenType::Enum);
  if (!PeekToken()->is_name())
    return Error(ErrorCode::SyntaxEnumDeclNameInvalid);
  auto const enum_name = ConsumeToken();
  if (FindMember(enum_name))
    Error(ErrorCode::SyntaxEnumDeclNameDuplicate);
  auto const enum_decl = factory()->NewEnum(namespace_body_, enum_modifiers,
                                            enum_keyword, enum_name);
  AddMember(enum_decl);
  // TODO(eval1749) NYI EnumBase ::= ':' IntegralType
  if (!AdvanceIf(TokenType::LeftCurryBracket))
    return Error(ErrorCode::SyntaxEnumDeclLeftCurryBracket);
  while (PeekToken()->is_name()) {
    auto const member_name = ConsumeToken();
    auto member_value = static_cast<ast::Expression*>(nullptr);
    if (AdvanceIf(TokenType::Assign)) {
      if (ParseExpression())
        member_value = ConsumeExpression();
      else
        Error(ErrorCode::SyntaxEnumDeclExpression);
    }
    enum_decl->AddMember(
        factory()->NewEnumMember(enum_decl, member_name, member_value));
    if (PeekToken() == TokenType::RightCurryBracket)
      break;
    if (AdvanceIf(TokenType::Comma))
      continue;
  }
  if (!AdvanceIf(TokenType::RightCurryBracket))
    return Error(ErrorCode::SyntaxEnumDeclRightCurryBracket);
  return true;
}

bool Parser::ParseFunctionDecl() {
  return false;
}

//  NamespaceDecl ::= "namespace" QualifiedName Namespace ";"?
//  Namespace ::= "{" ExternAliasDirective* UsingDirective*
//                        NamespaceMemberDecl* "}"
bool Parser::ParseNamespaceDecl() {
  auto const namespace_keyword = ConsumeToken();
  DCHECK_EQ(namespace_keyword->type(), TokenType::Namespace);
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
    new_namespace = present->as<ast::Namespace>();
    if (!new_namespace)
      Error(ErrorCode::SyntaxNamespaceDeclNameDuplicate, simple_name);
  }
  if (!new_namespace) {
    new_namespace = factory()->NewNamespace(namespace_body_, namespace_keyword,
                                            simple_name);
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
    while (modifiers_->Add(PeekToken()))
      Advance();
    switch (PeekToken()->type()) {
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
    if (!PeekToken()->is_name())
      break;
    name_builder_->Add(ConsumeToken());
    if (!AdvanceIf(TokenType::Dot))
      return true;
  }
  return false;
}

// UsingDirective ::= AliasDef | ImportNamespace
// AliasDef ::= 'using' Name '='  NamespaceOrTypeName ';'
// ImportNamespace ::= 'using' QualfiedName ';'
bool Parser::ParseUsingDirectives() {
  DCHECK(namespace_body_->owner()->as<ast::Namespace>());
  while (auto const using_keyword = ConsumeTokenIf(TokenType::Using)) {
    if (!ParseNamespaceOrTypeName())
      continue;
    auto const thing = ConsumeType();
    if (AdvanceIf(TokenType::Assign)) {
      if (!thing->is<ast::NameReference>()) {
        Error(ErrorCode::SyntaxUsingDirectiveAlias);
        continue;
      }
      auto const alias_name = thing->as<ast::NameReference>()->name();
      if (auto const present = namespace_body_->FindAlias(alias_name)) {
        Error(ErrorCode::SyntaxUsingDirectiveDuplicate, alias_name,
              present->keyword());
        continue;
      }
      if (ParseNamespaceOrTypeName()) {
        namespace_body_->AddAlias(factory()->NewAlias(
            namespace_body_, using_keyword, alias_name, ConsumeType()));
      }
    } else {
      auto const qualified_name = GetQualifiedNameToken(thing);
      if (!qualified_name) {
        Error(ErrorCode::SyntaxUsingDirectiveImport);
        continue;
      }
      if (auto const present = namespace_body_->FindImport(qualified_name)) {
        Error(ErrorCode::SyntaxUsingDirectiveDuplicate, qualified_name,
              present->reference()->token());
        continue;
      }
      namespace_body_->AddImport(
          factory()->NewImport(namespace_body_, using_keyword, thing));
    }
    if (!AdvanceIf(TokenType::SemiColon))
      Error(ErrorCode::SyntaxUsingDirectiveSemiColon);
  }
  return true;
}

Token* Parser::PeekToken() {
  if (token_)
    return token_;
  token_ = lexer_->GetToken();
  last_source_offset_ = token_->location().start_offset();
  return token_;
}

ast::NamespaceMember* Parser::ResolveMember(Token* name) const {
  DCHECK(name->is_name());
  for (auto runner = namespace_body_; runner; runner = runner->outer()) {
    if (auto const present = runner->FindAlias(name))
      return present;
    if (auto const present = runner->FindMember(name))
      return present;
  }
  return nullptr;
}

bool Parser::Run() {
  ParseCompilationUnit();
  return session_->errors().empty();
}

void Parser::ValidateClassModifiers() {
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

void Parser::ValidateEnumModifiers() {
  // TODO(eval1749) NYI validate enum modifier
}

void Parser::ValidateFieldModifiers() {
  // TODO(eval1749) NYI validate field modifier
}

void Parser::ValidateMethodModifiers() {
  // TODO(eval1749) NYI validate method modifier
}

}  // namespace compiler
}  // namespace elang
