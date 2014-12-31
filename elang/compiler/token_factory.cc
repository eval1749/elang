// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/token_factory.h"

#include "elang/base/zone.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Token
//
TokenFactory::TokenFactory(Zone* zone) : zone_(zone) {
}

TokenFactory::~TokenFactory() {
}

Token* TokenFactory::NewToken(const SourceCodeRange& source_range,
                              const TokenData& data) {
  return new(zone_) Token(source_range, data);
}

}  // namespace compiler
}  // namespace elang
