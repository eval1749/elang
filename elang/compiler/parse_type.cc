// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/array_type.h"
#include "elang/compiler/ast/constructed_type.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/member_access.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

// Just an alias of |ConsumeExpression()| for improving readbility.
ast::Expression* Parser::ConsumeType() {
  return ConsumeExpression();
}

// Type ::= ValueType | ReferenceType | TypeParameter
//
// TypeName ::= NamespaceOrTypeName
// NamespaceOrTypeName ::= Name TypeArgumentList? |
//                         QualifiedAliasMember |
//                         NamespaceOrTypeName '.' Name TypeArgumentList?
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
    ProduceType(factory()->NewNameReference(ConsumeToken()));
    return ParseTypePost();
  }

  if (!PeekToken()->is_name())
    return false;

  std::vector<ast::Expression*> type_names;
  type_names.push_back(factory()->NewNameReference(ConsumeToken()));
  for (;;) {
    if (AdvanceIf(TokenType::Dot)) {
      PeekToken();
      if (!PeekToken()->is_name())
        return Error(ErrorCode::SyntaxTypeDotNotName);
      type_names.push_back(factory()->NewNameReference(ConsumeToken()));
      continue;
    }
    if (auto const op_token = ConsumeTokenIf(TokenType::LeftAngleBracket)) {
      // TypeArgumentList ::= '<' Type (',' TypeName)* '>'
      ProduceType(factory()->NewMemberAccess(type_names));
      type_names.clear();
      std::vector<ast::Expression*> type_args;
      for (;;) {
        if (!ParseType())
          return false;
        type_args.push_back(ConsumeType());
        if (AdvanceIf(TokenType::Comma))
          continue;
        if (AdvanceIf(TokenType::RightAngleBracket)) {
          type_names.push_back(factory()->NewConstructedType(
              op_token, ConsumeType(), type_args));
          break;
        }
        Error(ErrorCode::SyntaxTypeComma);
        return false;
      }
      continue;
    }
    if (type_names.size() == 1)
      ProduceType(type_names.front());
    else
      ProduceType(factory()->NewMemberAccess(type_names));
    return ParseTypePost();
  }
}

// NullableType ::= NonNullableValueType '?'
// NonNullableValueType ::= EnumType | TypeName | SimpleType
//
// ArrayType ::= NonArrayType RankSpecifier*
// NonArrayType ::= ValueType | ClassType | InterfaceType | FunctionType |
//                  TypeParameter
// RankSpecifier ::= '[' ','* ']'
bool Parser::ParseTypePost() {
  if (auto const optional_marker = ConsumeTokenIf(TokenType::OptionalType)) {
    ProduceType(factory()->NewUnaryOperation(optional_marker,
                                                   ConsumeType()));
  }
  if (PeekToken() != TokenType::LeftSquareBracket)
    return true;
  auto const element_type = ConsumeType();
  auto const op_token = ConsumeToken();
  std::vector<int> ranks;
  while (AdvanceIf(TokenType::LeftSquareBracket)) {
    auto rank = 1;
    while (AdvanceIf(TokenType::Comma))
      ++rank;
    if (!AdvanceIf(TokenType::RightSquareBracket)) {
      Error(ErrorCode::SyntaxTypeRightSquareBracket);
      return false;
    }
    ranks.push_back(rank);
  }
  ProduceType(factory()->NewArrayType(op_token, element_type, ranks));
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
      Error(ErrorCode::SyntaxClassDeclTypeParamInvalid);
  }
  return type_params;
}

void Parser::ProduceType(ast::Expression* type) {
  ProduceExpression(type);
}

}  // namespace compiler
}  // namespace elang
