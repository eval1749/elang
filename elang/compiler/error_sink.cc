// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/compiler/error_sink.h"

#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

ErrorSink::ErrorSink(Zone* zone) : zone_(zone) {
}

ErrorSink::~ErrorSink() {
}

void ErrorSink::AddError(ErrorCode error_code, Token* token) {
  AddError(token->location(), error_code, std::vector<Token*>{token});
}

void ErrorSink::AddError(ErrorCode error_code, Token* token1, Token* token2) {
  AddError(token1->location(), error_code, std::vector<Token*>{token1, token2});
}

void ErrorSink::AddError(const SourceCodeRange& location,
                         ErrorCode error_code) {
  AddError(location, error_code, std::vector<Token*>());
}

void ErrorSink::AddError(const SourceCodeRange& location,
                         ErrorCode error_code,
                         const std::vector<Token*>& tokens) {
  std::vector<ErrorData*>* list =
      error_code > ErrorCode::WarningCodeZero ? &warnings_ : &errors_;
  list->push_back(new (zone_) ErrorData(zone_, location, error_code, tokens));
  std::sort(
      list->begin(), list->end(), [](const ErrorData* a, const ErrorData* b) {
        return a->location().start_offset() < b->location().start_offset();
      });
}

}  // namespace compiler
}  // namespace elang
