// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/error_reporter.h"

#include "elang/lir/error_data.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literal_map.h"

namespace elang {
namespace lir {

ErrorReporter::ErrorReporter(Factory* factory) : factory_(factory) {
}

ErrorReporter::~ErrorReporter() {
}

Value ErrorReporter::AsValue(Instruction* instruction) {
  return factory_->literals()->RegisterInstruction(instruction);
}

void ErrorReporter::Error(ErrorCode error_code, Instruction* instruction) {
  Error(error_code, AsValue(instruction));
}

void ErrorReporter::Error(ErrorCode error_code,
                          Instruction* instruction,
                          int detail) {
  Error(error_code, AsValue(instruction), Value::SmallInt32(detail));
}

void ErrorReporter::Error(ErrorCode error_code,
                          Instruction* instruction,
                          Value detail) {
  Error(error_code, AsValue(instruction), detail);
}

void ErrorReporter::Error(ErrorCode error_code, Value value) {
  factory_->AddError(error_code, value, {});
}

void ErrorReporter::Error(ErrorCode error_code, Value value, Value detail) {
  factory_->AddError(error_code, value, {detail});
}

void ErrorReporter::Error(ErrorCode error_code,
                          Value value,
                          Value detail1,
                          Value detail2) {
  factory_->AddError(error_code, value, {detail1, detail2});
}

}  // namespace lir
}  // namespace elang
