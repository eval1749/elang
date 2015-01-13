// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TOKEN_FACTORY_H_
#define ELANG_COMPILER_TOKEN_FACTORY_H_

#include "base/macros.h"

namespace elang {
class Zone;
namespace compiler {

class SourceCodeRange;
class Token;
class TokenData;

//////////////////////////////////////////////////////////////////////
//
// TokenFactory
//
class TokenFactory {
 public:
  explicit TokenFactory(Zone* zone);
  ~TokenFactory();

  Token* NewToken(const SourceCodeRange& source_range, const TokenData& data);

 private:
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(TokenFactory);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TOKEN_FACTORY_H_
