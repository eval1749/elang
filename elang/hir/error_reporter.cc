// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/hir/error_reporter.h"

#include "elang/hir/error_data.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"

namespace elang {
namespace hir {

ErrorReporter::ErrorReporter(Factory* factory) : factory_(factory) {
}

ErrorReporter::~ErrorReporter() {
}

void ErrorReporter::Error(ErrorCode error_code, const Value* error_value) {
  factory_->AddError(error_code, error_value, std::vector<Thing*>{});
}

void ErrorReporter::Error(ErrorCode error_code,
                          const Value* value,
                          Thing* detail) {
  factory_->AddError(error_code, value, std::vector<Thing*>{detail});
}

void ErrorReporter::Error(ErrorCode error_code,
                          const Instruction* instruction,
                          int index) {
  factory_->AddError(error_code, instruction, {NewInt32(index)});
}

void ErrorReporter::Error(ErrorCode error_code,
                          const Instruction* instruction,
                          int index,
                          Thing* detail) {
  factory_->AddError(error_code, instruction, {NewInt32(index), detail});
}

Value* ErrorReporter::NewInt32(int32_t data) {
  return factory_->NewInt32Literal(data);
}

}  // namespace hir
}  // namespace elang
