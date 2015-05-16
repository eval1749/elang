// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/syntax/parser.h"

#include <string>
#include <vector>
#include <unordered_set>

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/modifiers_builder.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/syntax/lexer.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {
Token* MakeQualifiedNameToken(ast::Node* thing) {
  if (auto const name_reference = thing->as<ast::NameReference>())
    return name_reference->name();
  if (auto const type_name_reference = thing->as<ast::TypeNameReference>())
    return type_name_reference->name();
  if (auto const type_member_access = thing->as<ast::TypeMemberAccess>())
    return MakeQualifiedNameToken(type_member_access->reference());
  if (auto const member_access = thing->as<ast::MemberAccess>()) {
    return member_access->token();
  }
  return nullptr;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ContainerScope
//
Parser::ContainerScope::ContainerScope(Parser* parser,
                                       ast::BodyNode* new_container)
    : container_(parser->container_), parser_(parser) {
  parser->container_ = new_container;
}

Parser::ContainerScope::~ContainerScope() {
  parser_->container_ = container_;
}

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
// Parser
//
Parser::Parser(CompilationSession* session, CompilationUnit* compilation_unit)
    : CompilationSessionUser(session),
      compilation_unit_(compilation_unit),
      container_(session->ast_factory()->NewNamespaceBody(
          session->global_namespace_body(),
          session->global_namespace())),
      declaration_space_(nullptr),
      expression_(nullptr),
      last_source_offset_(0),
      lexer_(new Lexer(session, compilation_unit)),
      modifiers_(new ModifierParser(this)),
      statement_(nullptr),
      statement_scope_(nullptr),
      token_(nullptr) {
  session->global_namespace_body()->AddMember(container_);
}

Parser::~Parser() {
}

ast::Factory* Parser::factory() const {
  return session()->ast_factory();
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
  session()->AddError(error_code, token, token2);
  return false;
}

bool Parser::Error(ErrorCode error_code, Token* token) {
  DCHECK(token);
  session()->AddError(error_code, token);
  return false;
}

bool Parser::Error(ErrorCode error_code) {
  return Error(error_code, token_);
}

Token* Parser::NewUniqueNameToken(const base::char16* format) {
  return session()->NewUniqueNameToken(
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
bool Parser::ParseClass() {
  ValidateClassModifiers();
  // TODO(eval1749) Support partial class.
  auto const class_modifiers = modifiers_->Get();
  auto const class_keyword = ConsumeToken();
  auto const class_name = ConsumeToken();
  if (!class_name->is_name())
    return Error(ErrorCode::SyntaxClassName, class_name);

  auto clazz = static_cast<ast::Class*>(nullptr);
  auto const local = container_->FindMember(class_name);
  auto const global = container_->owner()->FindMember(class_name);
  if (auto const present = local ? local : global) {
    clazz = present->as<ast::Class>();
    if (!clazz) {
      Error(ErrorCode::SyntaxClassConflict, class_name, present->name());
    } else if (clazz->IsPartial()) {
      if (!class_modifiers.HasPartial()) {
        // Existing declaration has 'partial' but this doesn't.
        Error(ErrorCode::SyntaxClassPartial, class_name);
      } else if (clazz->modifiers() != class_modifiers) {
        Error(ErrorCode::SyntaxClassPartialModifiers, class_name);
      }
    } else if (class_modifiers.HasPartial()) {
      // Existing declaration has no 'partial' but this has.
      Error(ErrorCode::SyntaxClassPartial, class_name);
    } else {
      // Both existing and this declaration don't have 'partial'.
      Error(ErrorCode::SyntaxClassDuplicate, class_name, clazz->name());
    }
  }
  if (!clazz) {
    clazz = factory()->NewClass(container_->owner(), class_modifiers,
                                class_keyword, class_name);
    container_->owner()->AddNamedMember(clazz);
  }
  // TypeParameterList
  if (AdvanceIf(TokenType::LeftAngleBracket))
    ParseTypeParameterList();

  // ClassBase
  std::vector<ast::Type*> base_class_names;
  if (AdvanceIf(TokenType::Colon)) {
    while (ParseNamespaceOrTypeName()) {
      base_class_names.push_back(ConsumeType());
      if (!AdvanceIf(TokenType::Comma))
        break;
    }
  }

  auto const class_body =
      factory()->NewClassBody(container_, clazz, base_class_names);
  container_->AddMember(class_body);
  container_->AddNamedMember(clazz);

  ContainerScope container_scope(this, class_body);

  if (class_modifiers.HasExtern()) {
    if (!AdvanceIf(TokenType::SemiColon))
      Error(ErrorCode::SyntaxClassSemiColon);
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
    return Error(ErrorCode::SyntaxClassLeftCurryBracket);

  for (;;) {
    modifiers_->Reset();
    while (modifiers_->Add(PeekToken()))
      Advance();

    switch (PeekToken()->type()) {
      case TokenType::Class:
      case TokenType::Interface:
      case TokenType::Struct:
        ParseClass();
        continue;
      case TokenType::Enum:
        ParseEnum();
        continue;
      case TokenType::Function:
        ParseFunction();
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
      ProduceTypeNameReference(var_keyword);
    } else if (!ParseType()) {
      return Error(ErrorCode::SyntaxClassRightCurryBracket);
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
      ParseMethod(member_modifiers, member_type, member_name, type_parameters);
      continue;
    }
    if (AdvanceIf(TokenType::LeftParenthesis)) {
      ParseMethod(member_modifiers, member_type, member_name, {});
      continue;
    }

    // FieldDecl ::= Type Name ('=' Expression)? ';'
    //
    if (auto const present = container_->FindMember(member_name)) {
      if (present->is<ast::Field>()) {
        Error(ErrorCode::SyntaxClassMemberDuplicate, member_name,
              present->name());
      } else {
        Error(ErrorCode::SyntaxClassMemberConflict, member_name,
              present->name());
      }
    }
    ValidateFieldModifiers();
    if (AdvanceIf(TokenType::Assign)) {
      if (!ParseExpression())
        return false;
      auto const field =
          factory()->NewField(class_body, member_modifiers, member_type,
                              member_name, ConsumeExpression());
      class_body->AddMember(field);
      class_body->AddNamedMember(field);
      clazz->AddNamedMember(field);
      if (!AdvanceIf(TokenType::SemiColon))
        Error(ErrorCode::SyntaxClassMemberSemiColon);
      continue;
    }

    // |var| field must have initial value.
    if (auto const name_ref = member_type->as<ast::TypeNameReference>()) {
      if (name_ref->name() == TokenType::Var)
        Error(ErrorCode::SyntaxClassMemberVarField, member_name);
    }

    auto const field = factory()->NewField(class_body, member_modifiers,
                                           member_type, member_name, nullptr);
    class_body->AddMember(field);
    class_body->AddNamedMember(field);
    clazz->AddNamedMember(field);
    if (!AdvanceIf(TokenType::SemiColon))
      Error(ErrorCode::SyntaxClassMemberSemiColon);
  }
}

// CompilationUnit ::=
//      ExternalAliasDirective
//      UsingDirective*
//      GlobalAttribute*
//      NamedNodeDecl*
void Parser::ParseCompilationUnit() {
  ParseUsingDirectives();
  ParseNamedNodes();
  if (!session()->errors().empty() || PeekToken() == TokenType::EndOfSource)
    return;
  Error(ErrorCode::SyntaxCompilationUnitInvalid, token_);
}

// EnumDecl := EnumModifier* "enum" Name EnumBase? "{" EnumField* "}"
// EnumBase ::= ':' IntegralType
// EnumField ::= Name ("=" Expression)? ","?
// EnumModifier ::= 'new' | 'public' | 'protected' | 'private'
void Parser::ParseEnum() {
  // TODO(eval1749) Validate EnumModifier
  auto const enum_modifiers = modifiers_->Get();
  auto const enum_keyword = ConsumeToken();
  DCHECK_EQ(enum_keyword->type(), TokenType::Enum);
  if (!PeekToken()->is_name()) {
    Error(ErrorCode::SyntaxEnumNameInvalid);
    token_ = session()->NewUniqueNameToken(token_->location(), L"enum%d");
  }
  auto const enum_name = ConsumeToken();
  auto const local = container_->FindMember(enum_name);
  auto const global = container_->owner()->FindMember(enum_name);
  if (auto const present = local ? local : global) {
    if (present->is<ast::Enum>())
      Error(ErrorCode::SyntaxEnumDuplicate, enum_name, present->name());
    else
      Error(ErrorCode::SyntaxEnumConflict, enum_name, present->name());
  }
  auto const enum_base =
      AdvanceIf(TokenType::Colon) && ParseType() ? ConsumeType() : nullptr;
  auto const enum_node = factory()->NewEnum(container_, enum_modifiers,
                                            enum_keyword, enum_name, enum_base);
  container_->AddMember(enum_node);
  container_->AddNamedMember(enum_node);
  container_->owner()->AddNamedMember(enum_node);
  // TODO(eval1749) NYI EnumBase ::= ':' IntegralType
  if (!AdvanceIf(TokenType::LeftCurryBracket))
    Error(ErrorCode::SyntaxEnumLeftCurryBracket);
  auto position = 0;
  while (PeekToken()->is_name()) {
    auto const member_name = ConsumeToken();
    auto member_value = static_cast<ast::Expression*>(nullptr);
    if (AdvanceIf(TokenType::Assign)) {
      if (ParseExpression())
        member_value = ConsumeExpression();
      else
        Error(ErrorCode::SyntaxEnumExpression);
    }
    auto const enum_member = factory()->NewEnumMember(enum_node, member_name,
                                                      position, member_value);
    enum_node->AddMember(enum_member);
    ++position;
    if (PeekToken() == TokenType::RightCurryBracket)
      break;
    if (AdvanceIf(TokenType::Comma))
      continue;
  }
  if (!AdvanceIf(TokenType::RightCurryBracket))
    Error(ErrorCode::SyntaxEnumRightCurryBracket);
}

void Parser::ParseFunction() {
  // TODO(eval1749) Implement 'function' parser.
}

//  NamespaceDecl ::= "namespace" QualifiedName Namespace ";"?
//  Namespace ::= "{" ExternAliasDirective* UsingDirective*
//                        NamedNodeDecl* "}"
bool Parser::ParseNamespace() {
  auto const namespace_keyword = ConsumeToken();
  DCHECK_EQ(namespace_keyword->type(), TokenType::Namespace);
  std::vector<Token*> names;
  do {
    if (!PeekToken()->is_name()) {
      if (!names.empty())
        Error(ErrorCode::SyntaxNamespaceName);
      if (PeekToken() != TokenType::LeftCurryBracket)
        Advance();
      break;
    }
    names.push_back(ConsumeToken());
  } while (AdvanceIf(TokenType::Dot));
  if (names.empty()) {
    Error(ErrorCode::SyntaxNamespaceAnonymous);
    names.push_back(NewUniqueNameToken(L"ns%d"));
  }
  return ParseNamespace(namespace_keyword, names, 0);
}

bool Parser::ParseNamespace(Token* namespace_keyword,
                            const std::vector<Token*>& names,
                            size_t index) {
  auto const ns_body = container_->as<ast::NamespaceBody>();
  DCHECK(ns_body);
  auto const name = names[index];
  ast::Namespace* new_namespace = nullptr;
  auto const local = container_->FindMember(name);
  auto const global = container_->owner()->FindMember(name);
  if (auto const present = local ? local : global) {
    new_namespace = present->as<ast::Namespace>();
    if (!new_namespace)
      Error(ErrorCode::SyntaxNamespaceConflict, name, present->keyword());
  }
  if (!new_namespace) {
    new_namespace =
        factory()->NewNamespace(ns_body->owner(), namespace_keyword, name);
    ns_body->owner()->AddNamedMember(new_namespace);
  }
  auto const new_ns_body = factory()->NewNamespaceBody(ns_body, new_namespace);
  ns_body->AddMember(new_ns_body);
  ns_body->AddNamedMember(new_namespace);
  ContainerScope container_scope(this, new_ns_body);
  if (index + 1 < names.size())
    return ParseNamespace(namespace_keyword, names, index + 1);
  if (!AdvanceIf(TokenType::LeftCurryBracket))
    return Error(ErrorCode::SyntaxNamespaceLeftCurryBracket);
  ParseUsingDirectives();
  ParseNamedNodes();
  if (!AdvanceIf(TokenType::RightCurryBracket))
    return false;
  AdvanceIf(TokenType::SemiColon);
  return true;
}

// NamedNodeDecl ::= NamespaceDecl | TypeDecl
// TypeDecl ::= ClassDecl | InterfaceDecl | StructDecl | EnumDecl |
//              FunctionDecl
void Parser::ParseNamedNodes() {
  auto is_namespace = container_->owner() != session()->global_namespace();
  auto skipping = false;
  for (;;) {
    modifiers_->Reset();
    while (modifiers_->Add(PeekToken()))
      Advance();
    switch (PeekToken()->type()) {
      case TokenType::Class:
      case TokenType::Interface:
      case TokenType::Struct:
        if (!ParseClass())
          return;
        continue;
      case TokenType::Enum:
        ParseEnum();
        continue;
      case TokenType::Function:
        ParseFunction();
        continue;
      case TokenType::Namespace:
        if (!ParseNamespace())
          return;
        continue;
      case TokenType::EndOfSource:
        return;
      case TokenType::RightCurryBracket:
        if (is_namespace)
          return;
        Advance();
        skipping = false;
        continue;
      case TokenType::SemiColon:
        if (skipping) {
          skipping = false;
          Advance();
          continue;
        }
        break;
      case TokenType::Using:
        skipping = true;
        break;
      default:
        if (skipping) {
          Advance();
          continue;
        }
        break;
    }
    if (is_namespace)
      Error(ErrorCode::SyntaxNamespaceInvalid, ConsumeToken());
    else
      Error(ErrorCode::SyntaxCompilationUnitInvalid, ConsumeToken());
  }
}

// UsingDirective ::= AliasDef | ImportNamespace
// AliasDef ::= 'using' Name '='  NamespaceOrTypeName ';'
// ImportNamespace ::= 'using' QualfiedName ';'
void Parser::ParseUsingDirectives() {
  auto const ns_body = container_->as<ast::NamespaceBody>();
  DCHECK(ns_body);
  while (auto const using_keyword = ConsumeTokenIf(TokenType::Using)) {
    if (!ParseNamespaceOrTypeName()) {
      AdvanceIf(TokenType::SemiColon);
      continue;
    }
    auto const thing = ConsumeType();
    if (AdvanceIf(TokenType::Assign)) {
      // AliasDef ::= 'using' Name '='  NamespaceOrTypeName ';'
      auto const type_name_ref = thing->as<ast::TypeNameReference>();
      if (!type_name_ref) {
        Error(ErrorCode::SyntaxUsingDirectiveAlias);
        AdvanceIf(TokenType::SemiColon);
        continue;
      }
      auto is_valid = true;
      auto const alias_name = type_name_ref->name();
      // Note: 'using' directive comes before other declarations. We don't
      // need to use enclosing namespace's |FindMember()|.
      if (auto const present = ns_body->FindMember(alias_name)) {
        is_valid = false;
        Error(ErrorCode::SyntaxUsingDirectiveDuplicate, alias_name,
              present->name());
      }
      if (ParseNamespaceOrTypeName()) {
        auto const reference = ConsumeType();
        if (is_valid) {
          auto const alias = factory()->NewAlias(ns_body, using_keyword,
                                                 alias_name, reference);
          ns_body->AddNamedMember(alias);
          ns_body->AddMember(alias);
        }
      }
    } else {
      // ImportNamespace ::= 'using' QualfiedName ';'
      auto const qualified_name = MakeQualifiedNameToken(thing);
      if (!qualified_name) {
        Error(ErrorCode::SyntaxUsingDirectiveImport, thing->token());
        AdvanceIf(TokenType::SemiColon);
        continue;
      }
      if (auto const import = ns_body->FindImport(qualified_name)) {
        Error(ErrorCode::SyntaxUsingDirectiveDuplicate, qualified_name,
              import->reference()->token());
      } else {
        ns_body->AddImport(factory()->NewImport(ns_body, using_keyword, thing));
      }
    }
    if (!AdvanceIf(TokenType::SemiColon))
      Error(ErrorCode::SyntaxUsingDirectiveSemiColon);
  }
}

Token* Parser::PeekToken() {
  if (token_)
    return token_;
  token_ = lexer_->GetToken();
  last_source_offset_ = token_->location().start_offset();
  if (token_->is_left_bracket()) {
    delimiters_.push_back(token_);
    return token_;
  }

  if (!token_->is_right_bracket())
    return token_;

  Token* left_bracket = nullptr;
  for (auto it = delimiters_.rbegin(); it != delimiters_.rend(); ++it) {
    auto const delimiter = *it;
    if (!delimiter->is_left_bracket())
      continue;
    if (token_ == delimiter->right_bracket()) {
      if (left_bracket)
        Error(ErrorCode::SyntaxBracketNotClosed, left_bracket, token_);
      while (delimiters_.back() != delimiter)
        delimiters_.pop_back();
      delimiters_.pop_back();
      return token_;
    }
    left_bracket = delimiter;
  }

  if (left_bracket)
    Error(ErrorCode::SyntaxBracketNotClosed, left_bracket, token_);
  else
    Error(ErrorCode::SyntaxBracketExtra);
  return token_;
}

void Parser::Run() {
  ParseCompilationUnit();
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
          Error(ErrorCode::SyntaxClassModifier, token);
        else
          has_inheritance = true;
        break;
      case TokenType::Private:
      case TokenType::Protected:
      case TokenType::Public:
        if (has_accessibility)
          Error(ErrorCode::SyntaxClassModifier, token);
        else
          has_accessibility = true;
        break;
      case TokenType::Virtual:
      case TokenType::Volatile:
        Error(ErrorCode::SyntaxClassModifier, token);
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
