// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/token.h"

#include "base/numerics/safe_conversions.h"
#include "elang/compiler/token_data.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Token
//
Token::Token(const SourceCodeRange& location, const TokenData& data)
    : data_(data), location_(location) {
}

bool Token::operator==(const Token& other) const {
  return location_ == other.location_ && data_ == other.data_;
}

bool Token::operator!=(const Token& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const Token& token) {
  return ostream << token.data();
}

std::ostream& operator<<(std::ostream& ostream, const Token* token) {
  if (token)
    return ostream << *token;
  return ostream << "(null)";
}

int16_t Token::int16_data() const {
  return base::checked_cast<int16_t>(uint64_data());
}

int32_t Token::int32_data() const {
  return base::checked_cast<int32_t>(uint64_data());
}

int64_t Token::int64_data() const {
  return base::checked_cast<int64_t>(uint64_data());
}

int8_t Token::int8_data() const {
  return base::checked_cast<int8_t>(uint64_data());
}

uint16_t Token::uint16_data() const {
  return base::checked_cast<uint16_t>(uint64_data());
}

uint32_t Token::uint32_data() const {
  return base::checked_cast<uint32_t>(uint64_data());
}

uint8_t Token::uint8_data() const {
  return base::checked_cast<uint8_t>(uint64_data());
}

}  // namespace compiler
}  // namespace elang
