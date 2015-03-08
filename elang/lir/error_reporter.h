// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ERROR_REPORTER_H_
#define ELANG_LIR_ERROR_REPORTER_H_

#include "base/basictypes.h"

namespace elang {
namespace lir {

enum class ErrorCode;
class ErrorData;
class Factory;
class Instruction;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// ErrorReporter
//
class ErrorReporter {
 protected:
  explicit ErrorReporter(Factory* factory);
  ~ErrorReporter();

  // Validation errors
  void Error(ErrorCode error_code, Instruction* instruction);
  void Error(ErrorCode error_code, Instruction* instruction, int detail);
  void Error(ErrorCode error_code, Instruction* instruction, Value detail);
  void Error(ErrorCode error_code, Value value);
  void Error(ErrorCode error_code, Value value, Value detail);
  void Error(ErrorCode error_code, Value value, Value detail1, Value detail2);

 private:
  Value AsValue(Instruction* instruction);

  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(ErrorReporter);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ERROR_REPORTER_H_
