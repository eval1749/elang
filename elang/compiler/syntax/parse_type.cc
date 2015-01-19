// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <vector>

#include "elang/compiler/syntax/parser.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

ast::Type* Parser::ConsumeExpressionAsType() {
  auto const expression = ConsumeExpressionOrType();
  if (auto const type = expression->as<ast::Type>())
    return type;
  if (auto const node = expression->as<ast::MemberAccess>())
    return factory()->NewTypeMemberAccess(node);
  if (auto const node = expression->as<ast::NameReference>())
    return factory()->NewTypeNameReference(node);
  Error(ErrorCode::SyntaxTypeNotType, expression->token());
  return factory()->NewInvalidType(expression);
}

ast::Type* Parser::ConsumeType() {
  auto const expression = ConsumeExpressionOrType();
  if (auto const type = expression->as<ast::Type>())
    return type;
  Error(ErrorCode::SyntaxTypeNotType, expression->token());
  return factory()->NewInvalidType(expression);
}

bool Parser::MaybeType(ast::Expression* maybe_type) const {
  return maybe_type->is<ast::Type>() || MaybeTypeName(maybe_type);
}

bool Parser::MaybeTypeName(ast::Expression* maybe_type) const {
  return maybe_type->is<ast::MemberAccess>() ||
         maybe_type->is<ast::NameReference>();
}

ast::Type* Parser::NewTypeNameReference(Token* name) {
  DCHECK(name->is_name() || name->is_keyword());
  return factory()->NewTypeNameReference(factory()->NewNameReference(name));
}

// ArrayType ::= Type ('[' ','* ']')+
void Parser::ParseArrayType(Token* bracket) {
  auto const element_type = ConsumeExpressionAsType();
  std::vector<int> ranks;
  do {
    auto rank = 1;
    while (AdvanceIf(TokenType::Comma))
      ++rank;
    if (!AdvanceIf(TokenType::RightSquareBracket))
      Error(ErrorCode::SyntaxTypeRightSquareBracket);
    ranks.push_back(rank);
  } while (AdvanceIf(TokenType::LeftSquareBracket));
  ProduceType(factory()->NewArrayType(bracket, element_type, ranks));
}

// NamespaceOrTypeName ::=
//   Name TypeArgumentList |
//   QualifiedAliasMember |
//   NamespaceOrTypeName '.' Name TypeArgumentList
bool Parser::ParseNamespaceOrTypeName() {
  enum class State {
    ConstructedType,
    Dot,
    Finish,
    Name,
    Start,
  };

  if (!PeekToken()->is_name()) {
    Error(ErrorCode::SyntaxTypeName);
    return false;
  }
  std::vector<ast::Expression*> names;
  auto state = State::Start;
  for (;;) {
    switch (state) {
      case State::ConstructedType:
        if (AdvanceIf(TokenType::Dot)) {
          state = State::Dot;
          continue;
        }
        state = State::Finish;
        continue;
      case State::Dot:
      case State::Start:
        if (!PeekToken()->is_name()) {
          Error(ErrorCode::SyntaxTypeName);
          return false;
        }
        names.push_back(factory()->NewNameReference(ConsumeToken()));
        state = State::Name;
        continue;
      case State::Name:
        if (AdvanceIf(TokenType::Dot)) {
          state = State::Dot;
          continue;
        }
        if (AdvanceIf(TokenType::LeftAngleBracket)) {
          // TypeArgumentList ::= '<' Type (',' TypeName)* '>'
          std::vector<ast::Type*> type_args;
          do {
            if (!ParseType())
              return false;
            type_args.push_back(ConsumeType());
          } while (AdvanceIf(TokenType::Comma));
          if (!AdvanceIf(TokenType::RightAngleBracket))
            Error(ErrorCode::SyntaxTypeRightAngleBracket);
          if (names.empty()) {
            Error(ErrorCode::SyntaxTypeTypeArgument);
          } else {
            ProduceTypeMemberAccess(names);
            names.clear();
            names.push_back(
                factory()->NewConstructedType(ConsumeType(), type_args));
          }
          state = State::ConstructedType;
          continue;
        }
        state = State::Finish;
        break;
      case State::Finish:
        DCHECK(!names.empty());
        ProduceTypeMemberAccess(names);
        return true;
    }
  }
}

// Type ::= ValueType | ReferenceType | TypeParameter
//
// TypeName ::= NamespaceOrTypeName
// ValueType ::= StructType | EnumType
// StructType ::= TypeName | SimpleType | NullableType
// SimpleType ::= NumericType | 'bool'
// NumericType ::= IntegralType | FloatingPointType
// IntegralType ::= 'int8' | 'int16' | 'int32' | 'int64' |
//                  'uint8' | 'uint16' | 'uint32' | 'uint64' | 'char'
// FloatingPointType ::= 'float32' | 'float64'
// EnumType ::= TypeName
// ReferenceType ::= ClassType | InterfaceType | ArrayType | FunctionType
bool Parser::ParseType() {
  if (PeekToken() == TokenType::Var) {
    // |var| isn't valid type name. Caller of |ParseType()| should handle |var|.
    return false;
  }

  if (PeekToken()->is_type_name()) {
    ProduceTypeNameReference(ConsumeToken());
    return ParseTypePost();
  }

  if (!ParseNamespaceOrTypeName())
    return false;
  return ParseTypePost();
}

// NullableType ::= NonNullableValueType '?'
// NonNullableValueType ::= EnumType | TypeName | SimpleType
//
// ArrayType ::= NonArrayType RankSpecifier*
// NonArrayType ::= ValueType | ClassType | InterfaceType | FunctionType |
//                  TypeParameter
// RankSpecifier ::= '[' ','* ']'
bool Parser::ParseTypePost() {
  if (auto const optional_marker = ConsumeTokenIf(TokenType::OptionalType))
    ProduceType(factory()->NewOptionalType(optional_marker, ConsumeType()));
  if (auto const bracket = ConsumeTokenIf(TokenType::LeftSquareBracket))
    ParseArrayType(bracket);
  return true;
}

// TypeParameterList ::= '<' TypeParameter (',' TypeParameter)* '>'
// TypeParameter ::= Attribute? Name
std::vector<Token*> Parser::ParseTypeParameterList() {
  std::vector<Token*> type_params;
  for (;;) {
    if (!PeekToken()->is_name())
      break;
    // TODO(eval1749) We should use |ast::TypeParameter| with |in|, |out| and
    // attribute list.
    type_params.push_back(ConsumeToken());
    if (AdvanceIf(TokenType::RightAngleBracket))
      break;
    if (!AdvanceIf(TokenType::Comma))
      Error(ErrorCode::SyntaxClassTypeParamInvalid);
  }
  return type_params;
}

ast::Expression* Parser::ProduceMemberAccess(
    const std::vector<ast::Expression*>& names) {
  DCHECK(!names.empty());
  DCHECK(!names.front()->is<ast::MemberAccess>());
  if (names.size() == 1)
    return ProduceExpressionOrType(names.front());
  // TODO(eval1749) We should use |base::string16| for creating name for
  // |MemberAccess|
  std::stringstream buffer;
  auto separator = "";
  for (auto const name : names) {
    buffer << separator << name->token();
    separator = ".";
  }
  auto const name_token = session()->NewToken(
      SourceCodeRange(compilation_unit_->source_code(),
                      names.front()->token()->location().start_offset(),
                      names.back()->token()->location().end_offset()),
      TokenData(TokenType::SimpleName,
                session()->NewAtomicString(base::UTF8ToUTF16(buffer.str()))));
  return ProduceExpression(factory()->NewMemberAccess(name_token, names));
}

ast::Type* Parser::ProduceType(ast::Type* type) {
  ProduceExpressionOrType(type);
  return type;
}

ast::Type* Parser::ProduceTypeMemberAccess(
    const std::vector<ast::Expression*>& names) {
  ProduceMemberAccess(names);
  auto const expression = ConsumeExpressionOrType();
  if (auto const type = expression->as<ast::Type>())
    return ProduceType(type);
  if (auto const node = expression->as<ast::MemberAccess>())
    return ProduceType(factory()->NewTypeMemberAccess(node));
  if (auto const node = expression->as<ast::NameReference>())
    return ProduceTypeNameReference(node);
  NOTREACHED();
  return ProduceType(factory()->NewInvalidType(expression));
}

ast::Type* Parser::ProduceTypeMemberAccess(ast::MemberAccess* node) {
  return ProduceType(factory()->NewTypeMemberAccess(node));
}

ast::Type* Parser::ProduceTypeNameReference(ast::NameReference* node) {
  return ProduceType(factory()->NewTypeNameReference(node));
}

ast::Type* Parser::ProduceTypeNameReference(Token* token) {
  return ProduceType(NewTypeNameReference(token));
}

}  // namespace compiler
}  // namespace elang
