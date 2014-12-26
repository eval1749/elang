// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/public/compiler_error_data.h"

#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

ErrorData::ErrorData(const SourceCodeRange& location,  ErrorCode error_code,
                     const std::vector<Token*>& tokens)
    : error_code_(error_code), source_code_location_(location),
      tokens_(tokens) {
}

ErrorData::~ErrorData() {
}

}  // namespace compiler
}  // namespace elang
