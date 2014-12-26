// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_token_factory_h)
#define INCLUDE_elang_compiler_token_factory_h

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
  private: std::vector<Token*> tokens_;

  public: TokenFactory();
  public: ~TokenFactory();

  public: Token* NewToken(const SourceCodeRange& source_range,
                          const TokenData& data);

  DISALLOW_COPY_AND_ASSIGN(TokenFactory);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_token_factory_h)

