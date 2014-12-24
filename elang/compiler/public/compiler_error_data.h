// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_public_error_data_h)
#define INCLUDE_elang_compiler_public_error_data_h

#include <vector>

#include "base/macros.h"
#include "elang/compiler/source_code_range.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

enum class ErrorCode;
class Token;

//////////////////////////////////////////////////////////////////////
//
// ErrorData
//
class ErrorData final {
  private: SourceCodeRange source_code_location_;
  private: ErrorCode error_code_;
  private: std::vector<Token> tokens_;

  public: ErrorData(const SourceCodeRange& location, ErrorCode error_code,
                    const std::vector<Token>& tokens_);
  public: ~ErrorData();

  public: ErrorCode error_code() const { return error_code_; }
  public: const std::vector<Token>& tokens() const { return tokens_; }

  DISALLOW_COPY_AND_ASSIGN(ErrorData);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_public_error_data_h)
