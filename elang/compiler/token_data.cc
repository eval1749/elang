// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/compiler/token_data.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/token_type.h"
#include "elang/hir/simple_name.h"

namespace elang {
namespace compiler {

namespace {
const char* const kTokenDetails[] = {
  #define T(name, string, details) details,
  TOKEN_LIST(T, T)
  #undef T
};

const char* GetTokenDetails(TokenType type) {
  return kTokenDetails[static_cast<size_t>(type)];
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TokenData
//
TokenData::TokenData(TokenType type) : type_(type) {
  data_.u64 = static_cast<uint64_t>(0);
}

TokenData::TokenData(float32_t f32) : type_(TokenType::Float32Literal) {
  data_.u64 = static_cast<uint64_t>(0);
  data_.f32 = f32;
}

TokenData::TokenData(float64_t f64) : type_(TokenType::Float64Literal) {
  data_.f64 = f64;
}

TokenData::TokenData(TokenType type, hir::SimpleName* simple_name)
    : type_(type) {
  data_.u64 = reinterpret_cast<uintptr_t>(simple_name);
}

TokenData::TokenData(hir::SimpleName* simple_name)
    : TokenData(TokenType::SimpleName, simple_name) {
}

TokenData::TokenData(base::StringPiece16* string)
    : type_(TokenType::StringLiteral) {
  DCHECK(has_string_data());
  data_.u64 = reinterpret_cast<uintptr_t>(string);
}

TokenData::TokenData(TokenType type, uint64_t u64) : type_(type) {
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

base::char16 TokenData::char_data() const {
  DCHECK_EQ(type_, TokenType::CharacterLiteral);
  return data_.ch;
}

bool TokenData::has_int_data() const {
  auto const detail = GetTokenDetails(type_)[1];
  return detail == 'I' || detail == 'U' || detail == 'C';
}

bool TokenData::has_simple_name() const {
  auto const detail = GetTokenDetails(type_)[0];
  return detail == 'N' || detail == 'K';
}

bool TokenData::has_string_data() const {
  return type_ == TokenType::StringLiteral;
}

float32_t TokenData::f32_data() const {
  DCHECK_EQ(type_, TokenType::Float32Literal);
  return data_.f32;
}

float64_t TokenData::f64_data() const {
  DCHECK_EQ(type_, TokenType::Float64Literal);
  return data_.f64;
}

int64_t TokenData::int64_data() const {
  DCHECK(has_int_data());
  return static_cast<int64_t>(data_.u64);
}

bool TokenData::is_contextual_keyword() const {
  return GetTokenDetails(type_)[0] == 'C';
}

bool TokenData::is_keyword() const {
  auto const detail = GetTokenDetails(type_)[0];
  return detail == 'C' || detail == 'K';
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

bool TokenData::is_type_name() const {
  return GetTokenDetails(type_)[1] == 'T';
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

hir::SimpleName* TokenData::simple_name() const {
  DCHECK(has_simple_name());
  return data_.name;
}

base::StringPiece16 TokenData::string_data() const {
  DCHECK(has_string_data());
  return *data_.str;
}

std::ostream& operator<<(std::ostream& ostream, const TokenData& token) {
  static const char* const kTokenTypeString[] = {
#   define V(name, string, details) string,
    TOKEN_LIST(V, V)
#   undef V
  };

  static char const xdigits[] = "0123456789ABCDEF";

  switch (token.type()) {
    case TokenType::CharacterLiteral: {
      auto const ch = token.char_data();
      char buffer[7];
      if (ch < ' ' || ch >= 0x7F || ch == '\\' || ch == '\'') {
          buffer[0] = '\\';
          buffer[1] = 'u';
          buffer[2] = xdigits[(ch >> 12) & 15];
          buffer[3] = xdigits[(ch >> 8) & 15];
          buffer[4] = xdigits[(ch >> 4) & 15];
          buffer[5] = xdigits[ch & 15];
          buffer[6] = 0;
      } else {
         buffer[0] = ch;
         buffer[1] = 0;
      }
      return ostream << "'" << buffer << "'";
    }
    case TokenType::Float32Literal:
      return ostream << token.f32_data() << "f";
    case TokenType::Float64Literal:
      return ostream << token.f64_data();
    case TokenType::Int32Literal:
      return ostream << token.int64_data();
    case TokenType::Int64Literal:
      return ostream << token.int64_data() << "l";
    case TokenType::UInt32Literal:
      return ostream << token.int64_data() << "u";
    case TokenType::UInt64Literal:
      return ostream << token.int64_data() << "lu";
    case TokenType::StringLiteral:
      ostream << " \"";
      for (auto const ch : token.string_data()) {
        char buffer[7];
        if (ch < ' ' || ch >= 0x7F || ch == '\\' || ch == '\"') {
          buffer[0] = '\\';
          buffer[1] = 'u';
          buffer[2] = xdigits[(ch >> 12) & 15];
          buffer[3] = xdigits[(ch >> 8) & 15];
          buffer[4] = xdigits[(ch >> 4) & 15];
          buffer[5] = xdigits[ch & 15];
          buffer[6] = 0;
        } else {
          buffer[0] = ch;
          buffer[1] = 0;
        }
        ostream << buffer;
      }
      return ostream << "\"";

    default:
      if (token.is_name() || token.is_keyword())
        return ostream << *token.simple_name();
      return ostream << kTokenTypeString[static_cast<int>(token.type())];
  }
}

}  // namespace compiler
}  // namespace elang
