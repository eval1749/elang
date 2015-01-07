// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <vector>

#include "elang/compiler/syntax/parser.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/array_type.h"
#include "elang/compiler/ast/constructed_type.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/member_access.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

// Just an alias of |ConsumeExpression()| for improving readability.
ast::Expression* Parser::ConsumeType() {
  return ConsumeExpression();
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
          std::vector<ast::Expression*> type_args;
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
            ProduceMemberAccess(names);
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
        ProduceMemberAccess(names);
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
    ProduceType(factory()->NewNameReference(ConsumeToken()));
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
  if (auto const optional_marker = ConsumeTokenIf(TokenType::OptionalType)) {
    ProduceType(factory()->NewUnaryOperation(optional_marker, ConsumeType()));
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

ast::Expression* Parser::ProduceMemberAccess(
    const std::vector<ast::Expression*>& names) {
  DCHECK(!names.empty());
  if (names.size() == 1)
    return ProduceType(names.back());
  // TODO(eval1749) We should use |base::string16| for creating name for
  // |MemberAccess|
  std::stringstream buffer;
  const char* separator = "";
  for (auto const name : names) {
    buffer << separator << name->token();
    separator = ".";
  }
  auto const name_token = session_->NewToken(
      SourceCodeRange(compilation_unit_->source_code(),
                      names.front()->token()->location().start_offset(),
                      names.back()->token()->location().end_offset()),
      TokenData(TokenType::SimpleName,
                session_->NewAtomicString(base::UTF8ToUTF16(buffer.str()))));
  return ProduceType(factory()->NewMemberAccess(name_token, names));
}

ast::Expression* Parser::ProduceType(ast::Expression* type) {
  return ProduceExpression(type);
}

}  // namespace compiler
}  // namespace elang
