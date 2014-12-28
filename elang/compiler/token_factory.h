// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TOKEN_FACTORY_H_
#define ELANG_COMPILER_TOKEN_FACTORY_H_

#include <vector>

#include "elang/compiler/token_data.h"

namespace elang {
namespace compiler {

class SourceCodeRange;

//////////////////////////////////////////////////////////////////////
//
// TokenFactory
//
class TokenFactory {
 public:
  TokenFactory();
  ~TokenFactory();

  Token* NewToken(const SourceCodeRange& source_range, const TokenData& data);

 private:
  std::vector<Token*> tokens_;

  DISALLOW_COPY_AND_ASSIGN(TokenFactory);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TOKEN_FACTORY_H_
