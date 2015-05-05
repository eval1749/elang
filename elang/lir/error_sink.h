// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ERROR_SINK_H_
#define ELANG_LIR_ERROR_SINK_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_user.h"

namespace elang {
namespace lir {

enum class ErrorCode;
class ErrorData;
class LiteralMap;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// ErrorSink
//
class ErrorSink final : public ZoneUser {
 public:
  ErrorSink(Zone* zone, LiteralMap* literals);
  ~ErrorSink();

  const std::vector<ErrorData*>& errors() const { return errors_; }

  void AddError(ErrorCode error_code,
                Value value,
                const std::vector<Value>& details);

 private:
  std::vector<ErrorData*> errors_;
  LiteralMap* const literals_;

  DISALLOW_COPY_AND_ASSIGN(ErrorSink);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ERROR_SINK_H_
