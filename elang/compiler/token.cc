// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/token.h"

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

}  // namespace compiler
}  // namespace elang
