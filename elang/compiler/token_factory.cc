// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/token_factory.h"

#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Token
//
TokenFactory::TokenFactory() {
}

TokenFactory::~TokenFactory() {
  for (auto const token : tokens_)
    delete token;
}

Token* TokenFactory::NewToken(const SourceCodeRange& source_range,
                              const TokenData& data) {
  auto const token = new Token(source_range, data);
  tokens_.push_back(token);
  return token;
}

}  // namespace compiler
}  // namespace elang
