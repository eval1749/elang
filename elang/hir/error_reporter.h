// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ERROR_REPORTER_H_
#define ELANG_HIR_ERROR_REPORTER_H_

#include "base/basictypes.h"

namespace elang {
namespace hir {

enum class ErrorCode;
class ErrorData;
class Factory;
class Instruction;
class Thing;
class Value;

//////////////////////////////////////////////////////////////////////
//
// ErrorReporter
//
class ErrorReporter {
 protected:
  explicit ErrorReporter(Factory* factory);
  ~ErrorReporter();

  // Validation errors
  void Error(ErrorCode error_code, const Value* value);
  void Error(ErrorCode error_code, const Value* value, Thing* detail);
  void Error(ErrorCode error_code, const Instruction* instruction, int index);
  void Error(ErrorCode error_code,
             const Instruction* instruction,
             int index,
             Thing* detail);

 private:
  Value* NewInt32(int32_t data);

  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(ErrorReporter);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ERROR_REPORTER_H_
