// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/compiler/token_data.h"

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/base/as_printable.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {
const char* const kTokenDetails[] = {
#define T(name, string, details) details,
    FOR_EACH_TOKEN(T, T)
#undef T
};

PredefinedName const kMappedTypeNames[] = {
#define Int Int32
#define V(Name) PredefinedName::Name,
    FOR_EACH_TYPE_KEYWORD(V)
#undef V
#undef Int
};

const char* GetTokenDetails(TokenType type) {
  return kTokenDetails[static_cast<size_t>(type)];
}

size_t GetTypeKeywordIndex(TokenType type) {
  return static_cast<size_t>(static_cast<int>(type) -
                             static_cast<int>(TokenType::Bool));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TokenData
//
TokenData::TokenData(TokenType type) : type_(type) {
  data_.u64 = static_cast<uint64_t>(0);
  DCHECK(!has_atomic_string());
}

TokenData::TokenData(float32_t f32) : type_(TokenType::Float32Literal) {
  data_.u64 = static_cast<uint64_t>(0);
  data_.f32 = f32;
}

TokenData::TokenData(float64_t f64) : type_(TokenType::Float64Literal) {
  data_.f64 = f64;
}

TokenData::TokenData(TokenType type, AtomicString* atomic_string)
    : type_(type) {
  DCHECK_GE(atomic_string->string().size(), 1u);
  data_.u64 = reinterpret_cast<uintptr_t>(atomic_string);
}

TokenData::TokenData(AtomicString* simple_name)
    : TokenData(TokenType::SimpleName, simple_name) {
}

TokenData::TokenData(base::StringPiece16* string)
    : type_(TokenType::StringLiteral) {
  DCHECK(has_string_data());
  data_.u64 = reinterpret_cast<uintptr_t>(string);
}

TokenData::TokenData(TokenType type, uint64_t u64) : type_(type) {
  DCHECK(!has_atomic_string());
  data_.u64 = u64;
}

TokenData::TokenData(const TokenData& other) : type_(other.type_) {
  data_.u64 = other.data_.u64;
}

TokenData::~TokenData() {
}

TokenData& TokenData::operator=(const TokenData& other) {
  data_.u64 = other.data_.u64;
  type_ = other.type_;
  return *this;
}

bool TokenData::operator==(const TokenData& other) const {
  if (type_ != other.type_)
    return false;
  if (data_.u64 == other.data_.u64)
    return true;
  switch (type_) {
    case TokenType::Float32Literal:
      return data_.f32 == other.data_.f32;
    case TokenType::Float64Literal:
      return data_.f64 == other.data_.f64;
    case TokenType::StringLiteral:
      return string_data() == other.string_data();
  }
  return false;
}

bool TokenData::operator!=(const TokenData& other) const {
  return !operator==(other);
}

AtomicString* TokenData::atomic_string() const {
  DCHECK(has_atomic_string());
  return data_.name;
}

bool TokenData::bool_data() const {
  if (type_ == TokenType::FalseLiteral)
    return false;
  if (type_ == TokenType::TrueLiteral)
    return true;
  NOTREACHED();
  return false;
}

base::char16 TokenData::char_data() const {
  DCHECK_EQ(type_, TokenType::CharacterLiteral);
  return data_.ch;
}

float32_t TokenData::f32_data() const {
  DCHECK_EQ(type_, TokenType::Float32Literal);
  return data_.f32;
}

float64_t TokenData::f64_data() const {
  DCHECK_EQ(type_, TokenType::Float64Literal);
  return data_.f64;
}

bool TokenData::has_atomic_string() const {
  auto const detail = GetTokenDetails(type_)[0];
  return detail == 'N' || detail == 'K' || detail == 'C';
}

bool TokenData::has_int_data() const {
  auto const detail = GetTokenDetails(type_)[1];
  return detail == 'I' || detail == 'U' || detail == 'C';
}

bool TokenData::has_string_data() const {
  return type_ == TokenType::StringLiteral;
}

int32_t TokenData::int32_data() const {
  return base::checked_cast<int32_t>(uint64_data());
}

int64_t TokenData::int64_data() const {
  return base::checked_cast<int64_t>(uint64_data());
}

bool TokenData::is_contextual_keyword() const {
  return GetTokenDetails(type_)[0] == 'C';
}

bool TokenData::is_keyword() const {
  auto const detail = GetTokenDetails(type_)[0];
  return detail == 'C' || detail == 'K';
}

bool TokenData::is_left_bracket() const {
  auto const details = GetTokenDetails(type_);
  return details[0] == 'P' && details[1] == 'L';
}

bool TokenData::is_literal() const {
  auto const details = GetTokenDetails(type_);
  return details[0] == 'L' || details[1] == 'L';
}

bool TokenData::is_name() const {
  auto const detail = GetTokenDetails(type_)[0];
  return detail == 'C' || detail == 'N';
}

bool TokenData::is_operator() const {
  return GetTokenDetails(type_)[0] == 'O';
}

bool TokenData::is_right_bracket() const {
  auto const details = GetTokenDetails(type_);
  return details[0] == 'P' && details[1] == 'R';
}

bool TokenData::is_type_name() const {
  // For speed, we use ordinal number instead of accessing details.
  return GetTypeKeywordIndex(type_) < arraysize(kMappedTypeNames);
}

PredefinedName TokenData::literal_type() const {
  static const PredefinedName kLiteralTypeNames[] = {
      PredefinedName::Char,
      PredefinedName::Bool,  // false
      PredefinedName::Float32,
      PredefinedName::Float64,
      PredefinedName::Int32,
      PredefinedName::Int64,
      PredefinedName::UInt32,
      PredefinedName::UInt64,
      PredefinedName::String,
      PredefinedName::Bool,  // true
  };
  auto const index = static_cast<size_t>(
      static_cast<int>(type_) - static_cast<int>(TokenType::CharacterLiteral));
  DCHECK_LT(index, arraysize(kLiteralTypeNames)) << type_;
  return kLiteralTypeNames[index];
}

PredefinedName TokenData::mapped_type_name() const {
  DCHECK(is_type_name());
  return kMappedTypeNames[GetTypeKeywordIndex(type_)];
}

// '0', '1', and '2' come from |ExpressionCategory::Unary| in
// "parser_expression.cc".
int TokenData::precedence() const {
  auto const details = kTokenDetails[static_cast<size_t>(type_)];
  if (is_operator())
    return details[1] - 'a' + 2;
  if (is_name() || is_literal())
    return 1;
  if (is_keyword() && details[1] == 'L')
    return 1;
  return 0;
}

TokenType TokenData::right_bracket() const {
  if (type_ == TokenType::LeftAngleBracket)
    return TokenType::RightAngleBracket;
  if (type_ == TokenType::LeftCurryBracket)
    return TokenType::RightCurryBracket;
  if (type_ == TokenType::LeftParenthesis)
    return TokenType::RightParenthesis;
  if (type_ == TokenType::LeftSquareBracket)
    return TokenType::RightSquareBracket;
  NOTREACHED();
  return TokenType::Illegal;
}

base::StringPiece16 TokenData::string_data() const {
  DCHECK(has_string_data());
  return *data_.str;
}

uint32_t TokenData::uint32_data() const {
  return base::checked_cast<uint32_t>(uint64_data());
}

uint64_t TokenData::uint64_data() const {
  DCHECK(has_int_data());
  return data_.u64;
}

std::ostream& operator<<(std::ostream& ostream, const TokenData& token) {
  static const char* const kTokenTypeString[] = {
#define V(name, string, details) string,
      FOR_EACH_TOKEN(V, V)
#undef V
  };

  switch (token.type()) {
    case TokenType::CharacterLiteral:
      return ostream << "'" << AsPrintable(token.char_data(), '\'') << "'";
    case TokenType::Float32Literal:
      return ostream << token.f32_data() << "f";
    case TokenType::Float64Literal:
      return ostream << token.f64_data();
    case TokenType::Int32Literal:
      return ostream << token.int32_data();
    case TokenType::Int64Literal:
      return ostream << token.int64_data() << "l";
    case TokenType::UInt32Literal:
      return ostream << token.uint32_data() << "u";
    case TokenType::UInt64Literal:
      return ostream << token.uint64_data() << "lu";
    case TokenType::StringLiteral:
      ostream << "\"";
      for (auto const ch : token.string_data())
        ostream << AsPrintable(ch, '"');
      return ostream << "\"";

    default:
      if (token.type() == TokenType::VerbatimName)
        ostream << '@';
      if (token.is_name() || token.is_keyword())
        return ostream << *token.atomic_string();
      return ostream << kTokenTypeString[static_cast<int>(token.type())];
  }
}

}  // namespace compiler
}  // namespace elang
