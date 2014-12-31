// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_PUBLIC_COMPILER_ERROR_DATA_H_
#define ELANG_COMPILER_PUBLIC_COMPILER_ERROR_DATA_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_object.h"
#include "elang/base/zone_vector.h"
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
class ErrorData final : public ZoneObject {
 public:
  ErrorCode error_code() const { return error_code_; }
  const SourceCodeRange& location() const { return source_code_location_; }
  const ZoneVector<Token*>& tokens() const { return tokens_; }

 private:
  friend class CompilationSession;

  ErrorData(Zone* zone,
            const SourceCodeRange& location,
            ErrorCode error_code,
            const std::vector<Token*>& tokens_);
  ~ErrorData() = delete;

  const SourceCodeRange source_code_location_;
  ErrorCode const error_code_;
  const ZoneVector<Token*> tokens_;

  DISALLOW_COPY_AND_ASSIGN(ErrorData);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_PUBLIC_COMPILER_ERROR_DATA_H_
