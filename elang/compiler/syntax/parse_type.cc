// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <vector>

#include "elang/compiler/syntax/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {
// This function should handle same token as |Parser::ParseTypeAfterName()|.
bool CanPartOfTypeReference(Token* token) {
  return token == TokenType::LeftAngleBracket || token == TokenType::Dot;
}
}  // namespace

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

ast::Type* Parser::ParseAndConsumeType() {
  ParseType();
  return ConsumeType();
}

// ArrayType ::= Type ('[' ','* ']')+
// Note: This function produces unbound array type. Bound array type are
// created by array initializer expression.
// Element type of T[A][B][C] is T[B][C], in other words element type of
// array type is removing left most rank specifier.
void Parser::ParseArrayType(Token* bracket) {
  auto element_type = ConsumeExpressionAsType();
  std::vector<std::vector<int>> dimensions_list;
  std::vector<Token*> brackets;
  brackets.push_back(bracket);
  for (;;) {
    std::vector<int> dimensions;
    do {
      dimensions.push_back(-1);
    } while (AdvanceIf(TokenType::Comma));
    if (!AdvanceIf(TokenType::RightSquareBracket))
      Error(ErrorCode::SyntaxTypeRightSquareBracket);
    DCHECK(!dimensions.empty());
    dimensions_list.push_back(dimensions);
    if (PeekToken() != TokenType::LeftSquareBracket)
      break;
    brackets.push_back(ConsumeToken());
  }
  DCHECK_EQ(brackets.size(), dimensions_list.size());
  auto type = element_type;
  while (!dimensions_list.empty()) {
    type =
        factory()->NewArrayType(brackets.back(), type, dimensions_list.back());
    brackets.pop_back();
    dimensions_list.pop_back();
  }
  ProduceType(type);
}

// NamespaceOrTypeName ::=
//   Name TypeArgumentList |
//   QualifiedAliasMember |
//   NamespaceOrTypeName '.' Name TypeArgumentList
void Parser::ParseNamespaceOrTypeName() {
  if (!PeekToken()->is_name()) {
    Error(ErrorCode::SyntaxTypeName);
    return ProduceInvalidType(PeekToken());
  }
  ProduceTypeNameReference(ConsumeToken());
  ParseTypeAfterName();
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
void Parser::ParseType() {
  if (PeekToken() == TokenType::Var) {
    // |var| isn't valid type name. Caller of |ParseType()| should handle |var|.
    Error(ErrorCode::SyntaxTypeName);
    return ProduceInvalidType(ConsumeToken());
  }

  if (PeekToken()->is_type_name()) {
    ProduceTypeNameReference(ConsumeToken());
    return ParseTypePost();
  }

  ParseNamespaceOrTypeName();
  ParseTypePost();
}

void Parser::ParseTypeAfterName() {
  enum class State {
    ConstructedType,
    Dot,
    Name,
  };

  auto state = State::Name;
  for (;;) {
    switch (state) {
      case State::ConstructedType:
        if (AdvanceIf(TokenType::Dot)) {
          state = State::Dot;
          continue;
        }
        return;
      case State::Dot:
        if (!PeekToken()->is_name()) {
          Error(ErrorCode::SyntaxTypeName);
          return ProduceType(factory()->NewInvalidType(ConsumeType()));
        }
        ProduceType(factory()->NewTypeMemberAccess(
            factory()->NewMemberAccess(ConsumeType(), ConsumeToken())));
        state = State::Name;
        continue;
      case State::Name:
        if (AdvanceIf(TokenType::Dot)) {
          state = State::Dot;
          continue;
        }
        if (AdvanceIf(TokenType::LeftAngleBracket)) {
          auto const generic_type = ConsumeType();
          // TypeArgumentList ::= '<' Type (',' TypeName)* '>'
          std::vector<ast::Type*> type_args;
          do {
            type_args.push_back(ParseAndConsumeType());
          } while (AdvanceIf(TokenType::Comma));
          if (!AdvanceIf(TokenType::RightAngleBracket))
            Error(ErrorCode::SyntaxTypeRightAngleBracket);
          ProduceType(factory()->NewConstructedType(
              factory()->NewConstructedName(generic_type, type_args)));
          state = State::ConstructedType;
          continue;
        }
        return;
    }
  }
}

// NullableType ::= NonNullableValueType '?'
// NonNullableValueType ::= EnumType | TypeName | SimpleType
//
// ArrayType ::= NonArrayType RankSpecifier*
// NonArrayType ::= ValueType | ClassType | InterfaceType | FunctionType |
//                  TypeParameter
// RankSpecifier ::= '[' ','* ']'
void Parser::ParseTypePost() {
  if (auto const optional_marker = ConsumeTokenIf(TokenType::OptionalType))
    ProduceType(factory()->NewOptionalType(optional_marker, ConsumeType()));
  if (auto const bracket = ConsumeTokenIf(TokenType::LeftSquareBracket))
    ParseArrayType(bracket);
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

Token* Parser::ParseVarTypeAndName() {
  if (PeekToken()->is_type_name()) {
    ParseType();
    return PeekToken()->is_name() ? ConsumeToken() : PeekToken();
  }
  if (!PeekToken()->is_name()) {
    ProduceInvalidType(PeekToken());
    return PeekToken();
  }
  auto const name = ConsumeToken();
  if (!CanPartOfTypeReference(PeekToken())) {
    ProduceType(factory()->NewTypeVariable(name));
    return name;
  }
  ProduceTypeNameReference(name);
  if (PeekToken()->is_name())
    return ConsumeToken();
  ParseTypeAfterName();
  return PeekToken()->is_name() ? ConsumeToken() : PeekToken();
}

void Parser::ProduceInvalidType(Token* token) {
  ProduceType(factory()->NewInvalidType(NewInvalidExpression(token)));
}

void Parser::ProduceType(ast::Type* type) {
  ProduceExpressionOrType(type);
}

void Parser::ProduceTypeNameReference(ast::NameReference* node) {
  ProduceType(factory()->NewTypeNameReference(node));
}

void Parser::ProduceTypeNameReference(Token* token) {
  ProduceType(NewTypeNameReference(token));
}

}  // namespace compiler
}  // namespace elang
