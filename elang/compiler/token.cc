// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/token.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {
const char* const kTokenDetails[] = {
  #define T(name, string, details) details,
  TOKEN_LIST(T, T)
  #undef T
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Token
//
Token::Token(const SourceCodeRange& location, TokenType type)
    : data_(static_cast<uint64_t>(0)), location_(location), type_(type) {
}

Token::Token(const SourceCodeRange& location, float32_t f32)
    : data_(f32), location_(location), type_(TokenType::Float32Literal) {
}

Token::Token(const SourceCodeRange& location, float64_t f64)
    : data_(f64), location_(location), type_(TokenType::Float64Literal) {
}

Token::Token(const SourceCodeRange& location, TokenType type,
             base::StringPiece16* string)
    : data_(reinterpret_cast<uintptr_t>(string)),
      location_(location),
      type_(type) {
  DCHECK(has_string_data());
}

Token::Token(const SourceCodeRange& location, TokenType type, uint64_t u64)
    : data_(u64), location_(location), type_(type) {
}

Token::Token(const Token& other)
    : data_(other.data_.u64), location_(other.location_), type_(other.type_) {
}

Token::Token()
  : Token(SourceCodeRange(), TokenType::None) {
}

Token::~Token() {
}

Token& Token::operator=(const Token& other) {
  data_.u64 = other.data_.u64;
  location_ = other.location_;
  type_ = other.type_;
  return *this;
}

base::char16 Token::char_data() const {
  DCHECK_EQ(type_, TokenType::CharacterLiteral);
  return data_.ch;
}

bool Token::has_int_data() const {
  auto const detail = kTokenDetails[static_cast<size_t>(type_)][1];
  return detail == 'I' || detail == 'U' || detail == 'C';
}

bool Token::has_string_data() const {
  return kTokenDetails[static_cast<size_t>(type_)][1] == 'S';
}

float32_t Token::f32_data() const {
  DCHECK_EQ(type_, TokenType::Float32Literal);
  return data_.f32;
}

float64_t Token::f64_data() const {
  DCHECK_EQ(type_, TokenType::Float64Literal);
  return data_.f64;
}

Token::SimpleNameId Token::id() const {
  DCHECK(is_name());
  return data_.str;
}

int64_t Token::int64_data() const {
  DCHECK(has_int_data());
  return static_cast<int64_t>(data_.u64);
}

bool Token::is_contextual_keyword() const {
  return kTokenDetails[static_cast<size_t>(type_)][0] == 'C';
}

bool Token::is_keyword() const {
  auto const detail = kTokenDetails[static_cast<size_t>(type_)][0];
  return detail == 'C' || detail == 'K';
}

bool Token::is_name() const {
  auto const detail = kTokenDetails[static_cast<size_t>(type_)][0];
  return detail == 'C' || detail == 'N';
}

base::StringPiece16 Token::string_data() const {
  DCHECK(has_string_data());
  return *data_.str;
}

std::ostream& operator<<(std::ostream& ostream, const Token& token) {
  static const char* const kTokenTypeString[] = {
    #define V(name, string, details) string,
    TOKEN_LIST(V, V)
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
        return ostream << base::UTF16ToUTF8(token.string_data().as_string());
      return ostream << kTokenTypeString[static_cast<int>(token.type())];
  }
}

}  // namespace compiler
}  // namespace elang
