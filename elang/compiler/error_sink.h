// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ERROR_SINK_H_
#define ELANG_COMPILER_ERROR_SINK_H_

#include <vector>

#include "base/macros.h"

namespace elang {
class Zone;
namespace compiler {
enum class ErrorCode;
class ErrorData;
class SourceCodeRange;
class Token;

//////////////////////////////////////////////////////////////////////
//
// ErrorSink
//
class ErrorSink {
 public:
  const std::vector<ErrorData*>& errors() const { return errors_; }
  const std::vector<ErrorData*>& warnings() const { return warnings_; }

  void AddError(ErrorCode error_code, Token* token);
  void AddError(ErrorCode error_code, Token* token1, Token* token2);
  // Lexer uses this.
  void AddError(const SourceCodeRange& location, ErrorCode error_code);

 protected:
  explicit ErrorSink(Zone* zone);
  ~ErrorSink();

 private:
  void AddError(const SourceCodeRange& location,
                ErrorCode error_code,
                const std::vector<Token*>& tokens);

  std::vector<ErrorData*> errors_;
  std::vector<ErrorData*> warnings_;
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(ErrorSink);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ERROR_SINK_H_
